#include "PlaneTextureBaker.hpp"

#include <algorithm>
#include <fstream>
#include <format>

#include "DirectXTex.h"
#include "Log.hpp"
#include "Utils.hpp"
#include "font/stb_truetype.h"

#undef min
#undef max

namespace {
    constexpr const char* kTextureRoot = "Assets/Resources/";
    constexpr const char* kFontPath = "Assets/Fonts/Satoshi-Variable.ttf";

    void BlendPixel(uint8_t* _dst, uint8_t _r, uint8_t _g, uint8_t _b, uint8_t _a) {
        const float srcA = _a / 255.f;
        if (srcA <= 0.f) return;

        const float dstA = _dst[3] / 255.f;
        const float outA = srcA + dstA * (1.f - srcA);
        if (outA <= 0.f) {
            _dst[0] = _dst[1] = _dst[2] = _dst[3] = 0;
            return;
        }

        _dst[0] = static_cast<uint8_t>(std::clamp((_r * srcA + _dst[0] * dstA * (1.f - srcA)) / outA, 0.f, 255.f));
        _dst[1] = static_cast<uint8_t>(std::clamp((_g * srcA + _dst[1] * dstA * (1.f - srcA)) / outA, 0.f, 255.f));
        _dst[2] = static_cast<uint8_t>(std::clamp((_b * srcA + _dst[2] * dstA * (1.f - srcA)) / outA, 0.f, 255.f));
        _dst[3] = static_cast<uint8_t>(std::clamp(outA * 255.f, 0.f, 255.f));
    }

    bool LoadImageAsRGBA8(const std::string& _textureKey, DirectX::ScratchImage& _outImage) {
        const std::string fullPath = kTextureRoot + _textureKey;
        const std::wstring pathW = Utils::Convert(fullPath);

        DirectX::ScratchImage loaded;
        const HRESULT hr = pathW.ends_with(L".dds")
            ? DirectX::LoadFromDDSFile(pathW.c_str(), DirectX::DDS_FLAGS_NONE, nullptr, loaded)
            : DirectX::LoadFromWICFile(pathW.c_str(), DirectX::WIC_FLAGS_FORCE_SRGB, nullptr, loaded);
        if (FAILED(hr)) {
            Log::Send(Log::Level::ERR, std::format("PlaneTextureBaker: failed to load image {}", _textureKey));
            return false;
        }

        if (loaded.GetMetadata().format == DXGI_FORMAT_R8G8B8A8_UNORM) {
            _outImage = std::move(loaded);
            return true;
        }

        const HRESULT convertHr = DirectX::Convert(*loaded.GetImage(0, 0, 0), DXGI_FORMAT_R8G8B8A8_UNORM,
            DirectX::TEX_FILTER_DEFAULT, DirectX::TEX_THRESHOLD_DEFAULT, _outImage);
        if (FAILED(convertHr)) {
            Log::Send(Log::Level::ERR, std::format("PlaneTextureBaker: failed to convert image format {}", _textureKey));
            return false;
        }
        return true;
    }

    void BlitTextureLayer(const PlaneTextureLayer& _layer, uint32_t _width, uint32_t _height, std::vector<uint8_t>& _pixels) {
        if (_layer.textureKey.empty()) return;

        DirectX::ScratchImage image;
        if (!LoadImageAsRGBA8(_layer.textureKey, image)) return;

        const int x0 = static_cast<int>(_layer.rectMin.x * static_cast<float>(_width));
        const int y0 = static_cast<int>(_layer.rectMin.y * static_cast<float>(_height));
        const int rectW = std::max(1, static_cast<int>((_layer.rectMax.x - _layer.rectMin.x) * static_cast<float>(_width)));
        const int rectH = std::max(1, static_cast<int>((_layer.rectMax.y - _layer.rectMin.y) * static_cast<float>(_height)));

        DirectX::ScratchImage resized;
        if (FAILED(DirectX::Resize(*image.GetImage(0, 0, 0), static_cast<size_t>(rectW), static_cast<size_t>(rectH),
                DirectX::TEX_FILTER_DEFAULT, resized))) {
            Log::Send(Log::Level::ERR, std::format("PlaneTextureBaker: failed to resize image {}", _layer.textureKey));
            return;
        }

        const DirectX::Image* src = resized.GetImage(0, 0, 0);
        for (int y = 0; y < rectH; ++y) {
            const int dstY = y0 + y;
            if (dstY < 0 || dstY >= static_cast<int>(_height)) continue;

            const uint8_t* srcRow = src->pixels + static_cast<size_t>(y) * src->rowPitch;
            for (int x = 0; x < rectW; ++x) {
                const int dstX = x0 + x;
                if (dstX < 0 || dstX >= static_cast<int>(_width)) continue;

                const uint8_t* srcPixel = srcRow + static_cast<size_t>(x) * 4;
                uint8_t* dstPixel = _pixels.data() + (static_cast<size_t>(dstY) * _width + dstX) * 4;
                BlendPixel(dstPixel,
                    static_cast<uint8_t>(srcPixel[0] * _layer.color.x),
                    static_cast<uint8_t>(srcPixel[1] * _layer.color.y),
                    static_cast<uint8_t>(srcPixel[2] * _layer.color.z),
                    static_cast<uint8_t>(srcPixel[3] * _layer.color.w));
            }
        }
    }

    std::vector<uint32_t> DecodeUtf8(const std::string& _text) {
        std::vector<uint32_t> codepoints;
        size_t i = 0;
        while (i < _text.size()) {
            const auto byte0 = static_cast<uint8_t>(_text[i]);
            uint32_t cp = 0;
            int extra = 0;
            if ((byte0 & 0x80) == 0x00) { cp = byte0; extra = 0; }
            else if ((byte0 & 0xE0) == 0xC0) { cp = byte0 & 0x1F; extra = 1; }
            else if ((byte0 & 0xF0) == 0xE0) { cp = byte0 & 0x0F; extra = 2; }
            else if ((byte0 & 0xF8) == 0xF0) { cp = byte0 & 0x07; extra = 3; }
            else { ++i; continue; }

            ++i;
            bool valid = true;
            for (int k = 0; k < extra && i < _text.size(); ++k, ++i) {
                const auto cont = static_cast<uint8_t>(_text[i]);
                if ((cont & 0xC0) != 0x80) { valid = false; break; }
                cp = (cp << 6) | (cont & 0x3F);
            }
            if (valid) codepoints.push_back(cp);
        }
        return codepoints;
    }

    const std::vector<uint8_t>* GetFontData() {
        static std::vector<uint8_t> fontData;
        static bool loaded = false;
        static bool loadFailed = false;

        if (loaded) return &fontData;
        if (loadFailed) return nullptr;

        std::ifstream file(kFontPath, std::ios::binary | std::ios::ate);
        if (!file) {
            loadFailed = true;
            Log::Send(Log::Level::ERR, std::format("PlaneTextureBaker: failed to open font {}", kFontPath));
            return nullptr;
        }

        const auto size = file.tellg();
        fontData.resize(static_cast<size_t>(size));
        file.seekg(0);
        file.read(reinterpret_cast<char*>(fontData.data()), size);

        loaded = true;
        return &fontData;
    }

    void BlitTextLayer(const PlaneTextureLayer& _layer, uint32_t _width, uint32_t _height, std::vector<uint8_t>& _pixels) {
        const std::vector<uint8_t>* fontData = GetFontData();
        if (!fontData) return;

        stbtt_fontinfo font;
        if (!stbtt_InitFont(&font, fontData->data(), 0)) {
            Log::Send(Log::Level::ERR, "PlaneTextureBaker: stbtt_InitFont failed");
            return;
        }

        const int x0 = static_cast<int>(_layer.rectMin.x * static_cast<float>(_width));
        const int y0 = static_cast<int>(_layer.rectMin.y * static_cast<float>(_height));
        const int rectH = std::max(1, static_cast<int>((_layer.rectMax.y - _layer.rectMin.y) * static_cast<float>(_height)));
        const int rectMaxX = static_cast<int>(_layer.rectMax.x * static_cast<float>(_width));

        const float scale = stbtt_ScaleForPixelHeight(&font, static_cast<float>(rectH));
        int ascent = 0, descent = 0, lineGap = 0;
        stbtt_GetFontVMetrics(&font, &ascent, &descent, &lineGap);
        const int penYBaseline = y0 + static_cast<int>(static_cast<float>(ascent) * scale);

        const uint8_t colorR = static_cast<uint8_t>(std::clamp(_layer.color.x, 0.f, 1.f) * 255.f);
        const uint8_t colorG = static_cast<uint8_t>(std::clamp(_layer.color.y, 0.f, 1.f) * 255.f);
        const uint8_t colorB = static_cast<uint8_t>(std::clamp(_layer.color.z, 0.f, 1.f) * 255.f);
        const float colorA = std::clamp(_layer.color.w, 0.f, 1.f);

        int penX = x0;
        for (const uint32_t codepoint : DecodeUtf8(_layer.text)) {
            if (penX >= rectMaxX) break;

            int advanceWidth = 0, leftBearing = 0;
            stbtt_GetCodepointHMetrics(&font, static_cast<int>(codepoint), &advanceWidth, &leftBearing);

            int glyphW = 0, glyphH = 0, xOff = 0, yOff = 0;
            uint8_t* bitmap = stbtt_GetCodepointBitmap(&font, scale, scale, static_cast<int>(codepoint), &glyphW, &glyphH, &xOff, &yOff);

            if (bitmap) {
                for (int gy = 0; gy < glyphH; ++gy) {
                    const int dstY = penYBaseline + yOff + gy;
                    if (dstY < 0 || dstY >= static_cast<int>(_height)) continue;

                    for (int gx = 0; gx < glyphW; ++gx) {
                        const int dstX = penX + xOff + gx;
                        if (dstX < 0 || dstX >= static_cast<int>(_width)) continue;

                        const uint8_t coverage = bitmap[gy * glyphW + gx];
                        if (coverage == 0) continue;

                        uint8_t* dstPixel = _pixels.data() + (static_cast<size_t>(dstY) * _width + dstX) * 4;
                        BlendPixel(dstPixel, colorR, colorG, colorB, static_cast<uint8_t>((coverage / 255.f) * colorA * 255.f));
                    }
                }
                stbtt_FreeBitmap(bitmap, nullptr);
            }

            penX += static_cast<int>(static_cast<float>(advanceWidth) * scale);
        }
    }
} // namespace

bool PlaneTextureBaker::Bake(const std::vector<PlaneTextureLayer>& _layers, uint32_t _width, uint32_t _height,
                              std::vector<uint8_t>& _outPixels) {
    if (_width == 0 || _height == 0) return false;

    _outPixels.assign(static_cast<size_t>(_width) * _height * 4, 0);

    std::vector<const PlaneTextureLayer*> order;
    order.reserve(_layers.size());
    for (const auto& layer : _layers) order.push_back(&layer);
    std::sort(order.begin(), order.end(),
        [](const PlaneTextureLayer* _a, const PlaneTextureLayer* _b) { return _a->zOrder < _b->zOrder; });

    for (const PlaneTextureLayer* layer : order) {
        if (layer->type == PlaneTextureLayer::Type::Texture) {
            BlitTextureLayer(*layer, _width, _height, _outPixels);
        } else {
            BlitTextLayer(*layer, _width, _height, _outPixels);
        }
    }

    return true;
}
