#include "Skybox.hpp"

#include "Pattern/Singleton.hpp"
#include "src/Texture/TextureManager.hpp"

void Skybox::Initialize(const std::string& _texture) {
    texture_ = _texture;

    TextureManager* tm = Singleton<TextureManager>::GetInstance();
    tm->Load(texture_);


}

void Skybox::CreateVertex() {

}
