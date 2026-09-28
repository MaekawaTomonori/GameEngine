#include "Texture/TextureHandle.hpp"

#include "Pattern/Singleton.hpp"
#include "src/Texture/TextureManager.hpp"

uint32_t TextureHandle::GetSrvIndex() const {
    return Singleton<TextureManager>::GetInstance()->GetSrvIndexOf(id_);
}

TextureState TextureHandle::GetState() const {
    return Singleton<TextureManager>::GetInstance()->GetStateOf(id_);
}

bool TextureHandle::IsReady() const {
    return GetState() == TextureState::Ready;
}
