#include "Model.hpp"

#include "Log.hpp"
#include "Singleton.hpp"
#include "Utils.hpp"
#include "Math/MathUtils.hpp"
#include "src/Mesh/Loader/MeshManager.hpp"

Model::Model()
    :common_(Singleton<ModelCommon>::GetInstance()),
    adapter_(common_->GetAdapter()),
    commandList_(adapter_->GetCommandList()),
    uuid_(Utils::GenerateUniqueId()){
}

void Model::Initialize(const std::string &_name) {
    if (!adapter_){
        Log::Send(Log::Level::ERR, "Adapter is null");
        return;
    }

    SetMesh(_name);

    wr_.Attach(adapter_->CreateBufferResource(sizeof(Transformation)));
    wr_->Map(0, nullptr, reinterpret_cast<void**>(&wd_));
    wd_->wvp = MathUtils::Matrix::MakeIdentity();
    wd_->world = MathUtils::Matrix::MakeIdentity();
    wd_->inverse = MathUtils::Matrix::MakeIdentity();
}

void Model::Update() {

}

void Model::Draw() const {
    if (!commandList_){
        Log::Send(Log::Level::ERR, "Command list is null");
        return;
    }
    if (!mesh_){
        Log::Send(Log::Level::ERR, "Mesh not set for model: " + meshName_);
        return;
    }

    common_->Draw();

    commandList_->SetGraphicsRootConstantBufferView(1, wr_->GetGPUVirtualAddress());
    //commandList_->SetGraphicsRootConstantBufferView(4, camera_->GetGPUVirtualAddress());

    mesh_->Draw();
}

void Model::Debug() {
}

void Model::SetMesh(const std::string &_name) {
    meshName_ = _name;

    mesh_ = Singleton<MeshManager>::GetInstance()->Load(_name);
}
