# ウィンドウリサイズ実装発展ロードマップ

このドキュメントは、Hazel Engineを参考にした当エンジンのウィンドウリサイズ機能の発展方針をまとめたものです。

## 📊 現状分析

### 現在の実装状況

#### ✅ 実装済み機能
- **ボーダレスフルスクリーン切り替え** (`Window::ToggleBorderless()`)
- **ビューポート・シザー矩形の更新** (`DirectXAdapter::UpdateWindowSize()`)
- **ImGui表示サイズの更新** (`Window::ToggleBorderless()` 内)
- **クライアント領域サイズ取得** (`WinApp::GetClientSize()`)

#### ❌ 未実装機能
- **WM_SIZEイベントのハンドリング** (現在は `WM_DESTROY` のみ)
- **SwapChainの動的リサイズ** (`IDXGISwapChain::ResizeBuffers()`)
- **深度ステンシルバッファの再作成**
- **レンダーターゲットビューの再作成**
- **イベント駆動アーキテクチャ**
- **最小化時の処理**
- **GPU同期管理**

### 現在の問題点

```cpp
// Window.cpp:41 - リサイズ不可のウィンドウスタイル
DWORD windowStyle = WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME & ~WS_MAXIMIZEBOX;
```

**問題**: ユーザーによる自由なウィンドウリサイズが無効化されている

**影響**:
- ウィンドウの境界をドラッグしてもサイズ変更できない
- 最大化ボタンも無効化されている
- ボーダレスフルスクリーンからの復帰時のみサイズ変更が発生

## 🎯 Hazel Engineからの学び

### 1. イベント駆動アーキテクチャ

**Hazelのアプローチ**:
```cpp
// GLFW callback → Custom Event → Layer Stack
glfwSetWindowSizeCallback(window, [](GLFWwindow* window, int width, int height) {
    WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
    WindowResizeEvent event(width, height);
    data.EventCallback(event);
});
```

**当エンジンへの適用**:
```cpp
// WindowProc で WM_SIZE をキャッチ → イベント発行 → システム更新
case WM_SIZE:
    if (wParam != SIZE_MINIMIZED) {
        int width = LOWORD(lParam);
        int height = HIWORD(lParam);
        // イベントシステムに通知
    }
    return 0;
```

### 2. SwapChain/Framebuffer リサイズパターン

**Hazelのパターン (OpenGL)**:
```cpp
void OpenGLFramebuffer::Resize(uint32_t width, uint32_t height) {
    m_Specification.Width = width;
    m_Specification.Height = height;
    Invalidate(); // 完全な再作成
}
```

**DirectX12での適応**:
```cpp
void DirectXAdapter::ResizeSwapChain(uint32_t width, uint32_t height) {
    // 1. GPU同期
    FlushCommandQueue();

    // 2. リソース解放
    for (auto& resource : swapChainResources_) {
        resource.reset();
    }
    depthStencil_.reset();

    // 3. SwapChainリサイズ
    swapChain_->ResizeBuffers(
        2,              // バッファ数
        width, height,  // 新しいサイズ
        DXGI_FORMAT_R8G8B8A8_UNORM,
        0
    );

    // 4. リソース再作成
    CreateRTV();
    CreateDSV();
    UpdateViewportAndScissor(width, height);
}
```

### 3. タイミング最適化

**Hazelで学んだ教訓**:
```cpp
// ❌ 間違い: OnUpdate の最後でリサイズ
void Application::OnUpdate() {
    // ... レンダリング処理
    if (needResize) Resize(); // フリッカーの原因
}

// ✅ 正しい: OnUpdate の最初でリサイズ
void Application::OnUpdate() {
    if (needResize) Resize(); // これでフリッカーなし
    // ... レンダリング処理
}
```

**参考**: [Hazel PR #268 - Fix black flicker on resize](https://github.com/TheCherno/Hazel/pull/268)

### 4. 最小化検出

**Hazelのパターン**:
```cpp
bool Application::OnWindowResize(WindowResizeEvent& e) {
    if (e.GetWidth() == 0 || e.GetHeight() == 0) {
        m_Minimized = true;
        return false; // レンダリングをスキップ
    }
    m_Minimized = false;

    Renderer::OnWindowResize(e.GetWidth(), e.GetHeight());
    return false;
}
```

**DirectX12での重要性**:
- 最小化時（0x0サイズ）に`Present()`を呼ぶとクラッシュする可能性
- SwapChainの最小サイズ制約違反を防ぐ

## 🗺️ 発展ロードマップ

### フェーズ1: 基礎イベントハンドリング (優先度: 🔴 高)

#### 1.1 WM_SIZEイベントの実装

**目的**: Windowsのリサイズイベントを検出する

**実装箇所**: `Window::WindowProc()`

**手法**:
```cpp
case WM_SIZE:
    if (wParam == SIZE_MINIMIZED) {
        // 最小化フラグを立てる
        isMinimized_ = true;
    } else {
        isMinimized_ = false;
        int width = LOWORD(lParam);
        int height = HIWORD(lParam);

        if (width > 0 && height > 0) {
            // リサイズコールバックを呼び出す
            if (onResizeCallback_) {
                onResizeCallback_(width, height);
            }
        }
    }
    return 0;
```

**追加メンバー**:
```cpp
class Window {
private:
    bool isMinimized_ = false;
    std::function<void(int, int)> onResizeCallback_;

public:
    void SetResizeCallback(std::function<void(int, int)> callback) {
        onResizeCallback_ = std::move(callback);
    }

    bool IsMinimized() const { return isMinimized_; }
};
```

#### 1.2 ウィンドウスタイルの変更

**目的**: リサイズ可能なウィンドウを有効化する

**実装箇所**: `Window::Create()`

**手法**:
```cpp
// リサイズ可能なウィンドウスタイル（オプション制御可能に）
DWORD windowStyle = isResizable_
    ? WS_OVERLAPPEDWINDOW  // 完全機能
    : WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME & ~WS_MAXIMIZEBOX; // 現在の動作
```

**コンストラクタパラメータ化**:
```cpp
struct WindowProperties {
    std::wstring title = L"Game Window";
    int width = 1280;
    int height = 720;
    bool resizable = true;      // ← 新規追加
    bool maximizable = true;    // ← 新規追加
};
```

### フェーズ2: DirectX リソース管理 (優先度: 🔴 高)

#### 2.1 GPU同期システム

**目的**: リサイズ前にGPUコマンド完了を保証する

**実装箇所**: `DirectXAdapter.cpp`

**手法**:
```cpp
void DirectXAdapter::FlushCommandQueue() {
    // 現在のフェンス値をインクリメント
    uint64_t targetFence = ++fenceValue_;

    // GPU にシグナルを送信
    ThrowIfFailed(cQueue_->Signal(fence_.Get(), targetFence));

    // GPUがこのフェンス値に到達するまで待機
    if (fence_->GetCompletedValue() < targetFence) {
        HANDLE eventHandle = CreateEventEx(nullptr, nullptr, 0, EVENT_ALL_ACCESS);
        ThrowIfFailed(fence_->SetEventOnCompletion(targetFence, eventHandle));
        WaitForSingleObject(eventHandle, INFINITE);
        CloseHandle(eventHandle);
    }
}
```

**注意点**:
- `Wait()` はフレーム単位の同期
- `FlushCommandQueue()` はリサイズなど特殊な操作用の完全同期

#### 2.2 SwapChain リサイズ実装

**目的**: SwapChainバッファを新しいサイズで再作成する

**実装箇所**: `DirectXAdapter.cpp`

**手法**:
```cpp
void DirectXAdapter::ResizeSwapChain(uint32_t width, uint32_t height) {
    // サイズが変わらない場合は何もしない
    if (width == windowSize_.first && height == windowSize_.second) {
        return;
    }

    // 最小サイズ制約
    width = std::max(width, 1u);
    height = std::max(height, 1u);

    // ステップ1: GPU完全同期
    FlushCommandQueue();

    // ステップ2: 既存リソースの解放
    for (auto& resource : swapChainResources_) {
        resource.reset();
    }
    rtvHandles_.clear();

    depthStencil_.reset();

    // ステップ3: SwapChain バッファリサイズ
    HRESULT hr = swapChain_->ResizeBuffers(
        2,                              // バッファ数
        width, height,                  // 新しいサイズ
        DXGI_FORMAT_R8G8B8A8_UNORM,    // フォーマット
        0                               // フラグ
    );

    if (FAILED(hr)) {
        Log::Send(Log::Level::ERR, "Failed to resize swap chain buffers");
        throw std::runtime_error("SwapChain resize failed");
    }

    // ステップ4: リソース再作成
    windowSize_ = {width, height};

    if (!CreateRTV()) {
        throw std::runtime_error("Failed to recreate RTV");
    }

    if (!CreateDSV()) {
        throw std::runtime_error("Failed to recreate DSV");
    }

    if (!CreateViewportAndScissor()) {
        throw std::runtime_error("Failed to recreate Viewport");
    }

    Log::Send(Log::Level::INFO,
        "SwapChain resized: " + std::to_string(width) + "x" + std::to_string(height));
}
```

**重要な実装詳細**:

1. **リソース解放順序**:
   ```cpp
   // 正しい順序:
   swapChainResources_.clear();  // RTVから参照されているリソース
   depthStencil_.reset();        // DSVから参照されているリソース
   // → この後 ResizeBuffers()
   ```

2. **エラーハンドリング**:
   ```cpp
   // DXGI_ERROR_DEVICE_REMOVED の検出
   if (hr == DXGI_ERROR_DEVICE_REMOVED) {
       HRESULT removalReason = device_->GetDeviceRemovedReason();
       Log::Send(Log::Level::ERR, "Device removed: " + std::to_string(removalReason));
       // デバイスの完全な再作成が必要
   }
   ```

3. **最小サイズ保証**:
   ```cpp
   // DirectX12 では 1x1 が最小サイズ
   width = std::max(width, 1u);
   height = std::max(height, 1u);
   ```

#### 2.3 最小化ハンドリング

**目的**: 最小化時のレンダリングエラーを防ぐ

**実装箇所**: `Framework::Run()`

**手法**:
```cpp
void Framework::Run() {
    while (winApp_->IsEnabled()) {
        // 最小化チェック
        if (window_->IsMinimized()) {
            // レンダリングをスキップ
            Sleep(100); // CPU使用率を下げる
            continue;
        }

        // 通常のレンダリングループ
        dxAdapter_->BeginFrame();
        // ... レンダリング処理
        dxAdapter_->EndFrame();
    }
}
```

### フェーズ3: イベントシステム統合 (優先度: 🟡 中)

#### 3.1 イベントクラスの設計

**目的**: 型安全でスケーラブルなイベントシステムを構築する

**実装箇所**: `include/Event/` (新規)

**手法**:

```cpp
// Event.hpp - 基底イベントクラス
enum class EventType {
    None = 0,
    WindowClose, WindowResize, WindowFocus, WindowLostFocus, WindowMoved,
    KeyPressed, KeyReleased, KeyTyped,
    MouseButtonPressed, MouseButtonReleased, MouseMoved, MouseScrolled
};

enum EventCategory {
    None = 0,
    EventCategoryApplication    = 1 << 0,
    EventCategoryInput          = 1 << 1,
    EventCategoryKeyboard       = 1 << 2,
    EventCategoryMouse          = 1 << 3,
    EventCategoryMouseButton    = 1 << 4
};

class Event {
public:
    virtual ~Event() = default;

    virtual EventType GetEventType() const = 0;
    virtual const char* GetName() const = 0;
    virtual int GetCategoryFlags() const = 0;

    bool IsInCategory(EventCategory category) const {
        return GetCategoryFlags() & category;
    }

    bool Handled = false;
};

// WindowResizeEvent.hpp
class WindowResizeEvent : public Event {
public:
    WindowResizeEvent(uint32_t width, uint32_t height)
        : width_(width), height_(height) {}

    uint32_t GetWidth() const { return width_; }
    uint32_t GetHeight() const { return height_; }

    EventType GetEventType() const override { return EventType::WindowResize; }
    const char* GetName() const override { return "WindowResize"; }
    int GetCategoryFlags() const override { return EventCategoryApplication; }

private:
    uint32_t width_, height_;
};
```

#### 3.2 イベントディスパッチャー

**目的**: イベントを型安全に配信する

**実装箇所**: `include/Event/EventDispatcher.hpp`

**手法**:
```cpp
class EventDispatcher {
public:
    EventDispatcher(Event& event) : event_(event) {}

    template<typename T, typename F>
    bool Dispatch(const F& func) {
        if (event_.GetEventType() == T::StaticType()) {
            event_.Handled |= func(static_cast<T&>(event_));
            return true;
        }
        return false;
    }

private:
    Event& event_;
};

// 使用例:
void Framework::OnEvent(Event& e) {
    EventDispatcher dispatcher(e);
    dispatcher.Dispatch<WindowResizeEvent>(
        [this](WindowResizeEvent& event) {
            return OnWindowResize(event);
        }
    );
}
```

#### 3.3 イベントコールバック統合

**目的**: WindowからFrameworkへイベントを伝播する

**実装箇所**: `Framework::Initialize()`

**手法**:
```cpp
void Framework::Initialize() {
    // ... 既存の初期化処理

    // ウィンドウリサイズコールバックを設定
    window_->SetResizeCallback([this](int width, int height) {
        WindowResizeEvent event(width, height);
        OnEvent(event);
    });
}

bool Framework::OnWindowResize(WindowResizeEvent& e) {
    if (e.GetWidth() == 0 || e.GetHeight() == 0) {
        // 最小化状態
        return false;
    }

    // DirectXAdapterにリサイズを通知
    dxAdapter_->ResizeSwapChain(e.GetWidth(), e.GetHeight());

    // カメラのアスペクト比を更新
    if (camera_) {
        float aspect = static_cast<float>(e.GetWidth()) / e.GetHeight();
        camera_->SetAspectRatio(aspect);
    }

    return false; // イベントを伝播させる
}
```

### フェーズ4: カメラ・ビューポート連携 (優先度: 🟡 中)

#### 4.1 カメラアスペクト比の自動更新

**目的**: リサイズ時にカメラプロジェクションを正しく調整する

**実装箇所**: `Camera/Camera.hpp`

**手法**:
```cpp
class Camera {
public:
    void SetAspectRatio(float aspect) {
        aspectRatio_ = aspect;
        UpdateProjectionMatrix();
    }

    void OnWindowResize(uint32_t width, uint32_t height) {
        SetAspectRatio(static_cast<float>(width) / height);
    }

private:
    void UpdateProjectionMatrix() {
        if (isPerspective_) {
            projectionMatrix_ = Matrix4x4::MakePerspectiveFov(
                fovY_, aspectRatio_, nearClip_, farClip_
            );
        } else {
            float halfWidth = orthoSize_ * aspectRatio_ * 0.5f;
            float halfHeight = orthoSize_ * 0.5f;
            projectionMatrix_ = Matrix4x4::MakeOrthographic(
                -halfWidth, halfWidth,
                -halfHeight, halfHeight,
                nearClip_, farClip_
            );
        }
    }

    float aspectRatio_ = 16.0f / 9.0f;
    bool isPerspective_ = true;
};
```

#### 4.2 ビューポート管理の拡張

**目的**: 複数ビューポート（エディタシーンビューなど）に対応する

**実装箇所**: `include/Renderer/Viewport.hpp` (新規)

**手法**:
```cpp
struct Viewport {
    float x = 0.0f;
    float y = 0.0f;
    float width = 1280.0f;
    float height = 720.0f;
    float minDepth = 0.0f;
    float maxDepth = 1.0f;

    D3D12_VIEWPORT ToD3D12() const {
        return D3D12_VIEWPORT{x, y, width, height, minDepth, maxDepth};
    }

    float GetAspectRatio() const {
        return width / height;
    }
};

class ViewportManager {
public:
    void SetMainViewport(const Viewport& viewport) {
        mainViewport_ = viewport;
        OnViewportChanged();
    }

    void OnWindowResize(uint32_t width, uint32_t height) {
        mainViewport_.width = static_cast<float>(width);
        mainViewport_.height = static_cast<float>(height);
        OnViewportChanged();
    }

private:
    void OnViewportChanged() {
        // カメラにアスペクト比を通知
        // レンダラーにビューポートを通知
    }

    Viewport mainViewport_;
};
```

### フェーズ5: エディタ統合 (優先度: 🟢 低)

#### 5.1 ImGui ビューポートウィジェット

**目的**: エディタ内でリサイズ可能なゲームビューを提供する

**実装箇所**: `EditorLayer::OnImGuiRender()`

**手法**:
```cpp
void EditorLayer::OnImGuiRender() {
    ImGui::Begin("Viewport");

    ImVec2 viewportSize = ImGui::GetContentRegionAvail();

    // ビューポートサイズが変わった場合
    if (viewportSize.x != lastViewportSize_.x ||
        viewportSize.y != lastViewportSize_.y) {

        lastViewportSize_ = viewportSize;

        // Framebufferをリサイズ
        framebuffer_->Resize(
            static_cast<uint32_t>(viewportSize.x),
            static_cast<uint32_t>(viewportSize.y)
        );

        // カメラを更新
        editorCamera_->SetViewportSize(viewportSize.x, viewportSize.y);
    }

    // レンダリング結果をImGuiイメージとして表示
    uint64_t textureID = framebuffer_->GetColorAttachmentRendererID();
    ImGui::Image((void*)textureID, viewportSize, ImVec2{0, 1}, ImVec2{1, 0});

    ImGui::End();
}
```

#### 5.2 マルチビューポート対応

**目的**: シーンビュー、ゲームビュー、カメラプレビューなど複数のビューを同時表示する

**実装箇所**: `Renderer/Framebuffer.hpp` (新規)

**手法**:
```cpp
class Framebuffer {
public:
    struct Specification {
        uint32_t Width = 1280;
        uint32_t Height = 720;
        uint32_t Samples = 1;
        bool SwapChainTarget = false;
    };

    Framebuffer(const Specification& spec);

    void Resize(uint32_t width, uint32_t height);
    void Bind() const;
    void Unbind() const;

    ID3D12Resource* GetColorAttachment() const { return colorAttachment_.Get(); }
    ID3D12Resource* GetDepthAttachment() const { return depthAttachment_.Get(); }

private:
    void Invalidate(); // 完全な再作成

    Specification spec_;
    Microsoft::WRL::ComPtr<ID3D12Resource> colorAttachment_;
    Microsoft::WRL::ComPtr<ID3D12Resource> depthAttachment_;
    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle_;
    D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle_;
};
```

## 📋 実装チェックリスト

### フェーズ1: 基礎イベントハンドリング
- [ ] `WM_SIZE` イベントハンドラー実装
- [ ] `Window::SetResizeCallback()` 追加
- [ ] 最小化フラグ (`isMinimized_`) 実装
- [ ] ウィンドウスタイルのパラメータ化 (`WindowProperties`)
- [ ] リサイズ可能/不可の切り替え機能

### フェーズ2: DirectX リソース管理
- [ ] `DirectXAdapter::FlushCommandQueue()` 実装
- [ ] `DirectXAdapter::ResizeSwapChain()` 実装
- [ ] SwapChainバッファの解放・再作成
- [ ] 深度ステンシルバッファの再作成
- [ ] RTVの再作成
- [ ] ビューポート・シザー矩形の更新
- [ ] 最小化時のレンダリングスキップ実装
- [ ] デバイス削除エラーハンドリング

### フェーズ3: イベントシステム統合
- [ ] `Event` 基底クラス設計
- [ ] `WindowResizeEvent` 実装
- [ ] `EventDispatcher` 実装
- [ ] `Framework::OnEvent()` 実装
- [ ] `Framework::OnWindowResize()` 実装
- [ ] イベントコールバックの統合

### フェーズ4: カメラ・ビューポート連携
- [ ] `Camera::SetAspectRatio()` 実装
- [ ] `Camera::OnWindowResize()` 実装
- [ ] 自動プロジェクション行列更新
- [ ] `Viewport` 構造体設計
- [ ] `ViewportManager` 実装
- [ ] カメラとビューポートの連携

### フェーズ5: エディタ統合
- [ ] `Framebuffer` クラス設計
- [ ] `Framebuffer::Resize()` 実装
- [ ] ImGuiビューポートウィジェット
- [ ] エディタカメラのビューポート対応
- [ ] マルチビューポートサポート

## 🎯 実装優先順位

### 即座に実装すべき (🔴 クリティカル)
1. **WM_SIZEハンドリング** - リサイズイベントの検出基盤
2. **GPU同期** - リソース破棄前の安全性確保
3. **SwapChainリサイズ** - DirectX12の中核機能
4. **最小化検出** - クラッシュ防止

### 次のステップ (🟡 重要)
5. **イベントシステム** - スケーラブルなアーキテクチャ
6. **カメラ連携** - 正しいアスペクト比維持

### 将来的な拡張 (🟢 オプション)
7. **Framebufferクラス** - エディタ開発の基盤
8. **マルチビューポート** - 高度なエディタ機能

## 📝 実装時の注意点

### DirectX12固有の考慮事項

1. **リソース解放順序の厳守**:
   ```cpp
   // 正しい順序:
   swapChainResources_.clear();     // 1. RTVが参照するリソース
   depthStencil_.reset();           // 2. DSVが参照するリソース
   swapChain_->ResizeBuffers(...);  // 3. SwapChainリサイズ
   CreateRTV();                     // 4. RTV再作成
   CreateDSV();                     // 5. DSV再作成
   ```

2. **GPU同期の完全性**:
   ```cpp
   // ❌ 不十分: 単純なフェンス待機
   Wait();

   // ✅ 正しい: コマンドキューの完全なフラッシュ
   FlushCommandQueue();
   ```

3. **最小サイズ制約**:
   ```cpp
   // DirectX12 は最小 1x1
   width = std::max(width, 1u);
   height = std::max(height, 1u);
   ```

4. **DXGI_ERROR_DEVICE_REMOVED の処理**:
   ```cpp
   if (hr == DXGI_ERROR_DEVICE_REMOVED) {
       // デバイス全体の再作成が必要
       // ログに詳細を記録
       // ユーザーに通知
   }
   ```

### パフォーマンス最適化

1. **不要なリサイズの回避**:
   ```cpp
   if (newWidth == currentWidth && newHeight == currentHeight) {
       return; // サイズが変わっていない場合は何もしない
   }
   ```

2. **リサイズスロットリング**:
   ```cpp
   // ドラッグ中の連続的なリサイズを遅延させる
   if (isDragging) {
       resizeTimer_.Reset();
       return;
   }

   // タイマー満了後に実際のリサイズを実行
   if (resizeTimer_.Elapsed() > 0.1f) {
       PerformResize();
   }
   ```

3. **リソースプールの活用**:
   ```cpp
   // 頻繁なリサイズに備えてリソースをプール
   class FramebufferPool {
       std::map<std::pair<uint32_t, uint32_t>, Framebuffer*> pool_;
   };
   ```

### デバッグとテスト

1. **リサイズ検証**:
   ```cpp
   #ifdef _DEBUG
   void ValidateSwapChain() {
       DXGI_SWAP_CHAIN_DESC1 desc;
       swapChain_->GetDesc1(&desc);
       assert(desc.Width == windowSize_.first);
       assert(desc.Height == windowSize_.second);
   }
   #endif
   ```

2. **ログ記録**:
   ```cpp
   Log::Send(Log::Level::INFO,
       "Resize: " + std::to_string(oldWidth) + "x" + std::to_string(oldHeight) +
       " → " + std::to_string(newWidth) + "x" + std::to_string(newHeight)
   );
   ```

3. **エッジケーステスト**:
   - 最小化 → 復元
   - 最大化
   - マルチモニター間の移動
   - DPI変更
   - 高速な連続リサイズ

## 🔗 参考リソース

### Hazel Engine
- **GitHub**: https://github.com/TheCherno/Hazel
- **重要なファイル**:
  - `Hazel/src/Platform/Windows/WindowsWindow.cpp` - WM_SIZEハンドリング
  - `Hazel/src/Hazel/Core/Application.cpp` - リサイズタイミング
  - `Hazel/src/Platform/OpenGL/OpenGLFramebuffer.cpp` - Invalidateパターン
- **参考PR**:
  - [#268 - Fix black flicker on resize](https://github.com/TheCherno/Hazel/pull/268)

### DirectX12 公式ドキュメント
- **IDXGISwapChain::ResizeBuffers**: https://docs.microsoft.com/en-us/windows/win32/api/dxgi/nf-dxgi-idxgiswapchain-resizebuffers
- **Handling Window Resizing**: https://docs.microsoft.com/en-us/windows/win32/direct3d12/handling-window-resizing

### 関連トピック
- **Frank Luna - "Introduction to 3D Game Programming with DirectX 12"** - Chapter 6: Drawing in Direct3D (Resizing the Window)
- **3D Graphics Community**: https://www.3dgep.com/learning-directx-12-2/#Resize_Event

## 🎓 学習ポイント

### Hazel から学ぶべきパターン

1. **完全な再作成 > インプレースリサイズ**
   - OpenGLの`Invalidate()`パターンは、DirectX12でも適用可能
   - 複雑な状態管理よりも、クリーンな再作成の方が安全

2. **タイミングが全て**
   - リサイズは`OnUpdate()`の**最初**
   - これにより1フレーム遅延を防ぎ、フリッカーを回避

3. **最小化は特殊ケース**
   - 0x0サイズは必ず検出してスキップ
   - レンダリングループの外側で処理

4. **イベント駆動の利点**
   - ポーリングよりもイベント駆動の方が効率的
   - 複数のシステム（カメラ、UI、レンダラー）が独立して反応可能

### DirectX12 特有の教訓

1. **GPU同期は妥協できない**
   - OpenGLは暗黙的に同期してくれる
   - DirectX12では明示的な`Flush`が必須

2. **リソース所有権の明確化**
   - SwapChainバッファは特殊（DXGIが所有）
   - 再作成前に必ずリセット

3. **デバイス削除は稀だが致命的**
   - ドライバクラッシュ、TDR、GPUオーバークロック失敗など
   - 検出して適切にログ記録、可能なら回復

## 📈 完成後の期待される効果

### ユーザー体験の向上
- ✅ 自由なウィンドウリサイズ
- ✅ スムーズなフルスクリーン切り替え
- ✅ 最小化・復元時のクラッシュなし
- ✅ 正しいアスペクト比の維持

### 開発体験の向上
- ✅ エディタビューポートのリサイズ対応
- ✅ 複数のプレビューウィンドウ
- ✅ リアルタイムプレビュー

### アーキテクチャの向上
- ✅ スケーラブルなイベントシステム
- ✅ 型安全なイベント配信
- ✅ モジュール間の疎結合
- ✅ テスト可能な設計

## 🚀 次のステップ

1. **フェーズ1から開始**: WM_SIZEハンドリングの実装
2. **段階的なテスト**: 各フェーズ完了後に動作確認
3. **既存機能の維持**: ボーダレスフルスクリーンは引き続きサポート
4. **継続的なリファクタリング**: Hazelのベストプラクティスを取り入れながら改善

---

**最終更新**: 2025-11-30
**参照エンジン**: Hazel Engine by TheCherno
**対象エンジン**: PortfolioGame Engine (DirectX12)
