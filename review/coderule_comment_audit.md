# Coderule Comment Audit Report

- Scope: include/**/*.hpp|h, src/**/*.hpp|h
- Reference: @docs/Coderule.md (Doxygen comment rule)
- Rule used: Use /** ... */ style comments; do not use /// style comments.

## 1. Comment Rule Violations (Usage of ///)

### .\include\Model.hpp
- L77 (triple-slash)
- L155 (triple-slash)
- L156 (triple-slash)

### .\src\PostProcess\Executor\PostProcessExecutor.hpp
- L92 (triple-slash)
- L134 (triple-slash @brief)
- L135 (triple-slash @param)
- L136 (triple-slash @param)
- L137 (triple-slash @param)
- L138 (triple-slash @return)

### .\src\PostProcess\IPostEffect.hpp
- L74 (triple-slash <summary>)
- L75 (triple-slash)
- L76 (triple-slash)
- L77 (triple-slash <returns>)
- L80 (triple-slash <summary>)
- L81 (triple-slash)
- L82 (triple-slash)
- L83 (triple-slash <returns>)
- L86 (triple-slash <summary>)
- L87 (triple-slash)
- L88 (triple-slash)
- L89 (triple-slash)
- L92 (triple-slash <summary>)
- L93 (triple-slash)
- L94 (triple-slash)
- L95 (triple-slash)
- L96 (triple-slash <returns>)
- L99 (triple-slash <summary>)
- L100 (triple-slash)
- L101 (triple-slash)
- L102 (triple-slash)
- L103 (triple-slash <returns>)
- L106 (triple-slash <summary>)
- L107 (triple-slash)
- L108 (triple-slash)
- L109 (triple-slash)
- L110 (triple-slash)
- L111 (triple-slash <returns>)
- L114 (triple-slash <summary>)
- L115 (triple-slash)
- L116 (triple-slash)
- L117 (triple-slash)
- L120 (triple-slash <summary>)
- L121 (triple-slash)
- L122 (triple-slash)
- L123 (triple-slash)
- L126 (triple-slash <summary>)
- L127 (triple-slash)
- L128 (triple-slash)
- L129 (triple-slash)

### .\src\Scene\SceneFactory.hpp
- L20 (triple-slash @brief)
- L21 (triple-slash @param)
- L22 (triple-slash @param)
- L25 (triple-slash @brief)
- L26 (triple-slash @param)
- L27 (triple-slash @return)
- L30 (triple-slash @brief)
- L31 (triple-slash @return)

### .\src\Scene\SceneSwitcher.hpp
- L56 (triple-slash @brief)
- L57 (triple-slash @param)
- L58 (triple-slash @param)

## 2. Header Function Declarations Missing Nearby Comments (Potential)

- Detection rule: no comment line (`//` or `/* ... */`) found in the nearest non-empty line before the declaration.
- Note: this is a heuristic and may include false positives.

### .\include\DebugUI.hpp
- L52: ~DebugUI();
- L75: bool& IsVisible(const std::string& _key);

### .\include\Framework.hpp
- L64: Framework();
- L65: ~Framework();

### .\include\IGame.hpp
- L21: IGame();
- L22: virtual ~IGame();
- L24: virtual void Initialize(GameEngine::Config& _config) = 0;

### .\include\Input.hpp
- L55: bool IsTrigger(BYTE _key) const;
- L84: void UpdateKeyboard();
- L85: void UpdateMouse();

### .\include\IScene.hpp
- L98: PostProcessExecutor* PostEffect() const;
- L99: ParticleSystem* Particle() const;

### .\include\Line.hpp
- L68: Line();
- L69: ~Line();

### .\include\Log.hpp
- L102: static void Debug(DebugUI* _debug);
- L136: static bool SendFromPanel();

### .\include\Model.hpp
- L93: Model();
- L94: ~Model();
- L159: std::string GetUniqueId();
- L161: Mesh* GetMesh() const;

### .\include\Sprite.hpp
- L82: Sprite();
- L83: ~Sprite();

### .\include\WatchDebugger.hpp
- L75: void AddReadOnly(const std::string& _label, std::function<std::string()> _getter);
- L76: void AddEditable(const std::string& _label, void* _ptr, EditType _type, const std::string& _jsonFile, const std::string& _jsonKey);
- L77: void AddGroupBegin(const std::string& _label);
- L78: void AddGroupEnd();
- L80: void RenderReadOnly(const Entry& _e);
- L81: void RenderEditable(Entry& _e);
- L82: void WriteToJson(const Entry& _e);
- L84: void SaveJsonFiles();
- L86: static WatchValue ReadFromPtr(void* _ptr, EditType _type);
- L175: WatchEdit(_label, _ptr, _jsonFile, _jsonKey);

### .\src\Camera\Camera.hpp
- L37: Camera();
- L107: void LookAt(const Vector3& _position);

### .\src\Camera\Controller\CameraController.hpp
- L16: void Initialize(float _ratio, DebugUI* _debug);
- L17: void Update() const;
- L18: void Debug();
- L20: Camera* GetActive() const;
- L21: Camera* Add(const std::string& _name = "") const;
- L22: Camera* SetActive(const std::string& _name);
- L25: void Load();
- L26: void Save() const;

### .\src\Camera\Director\CameraDirector.hpp
- L77: void Initialize(DebugUI* _debug);
- L78: void Update();
- L79: void Draw();
- L80: void Load(const std::string& _key);
- L81: void Run(const std::string& _key, bool _loop = false, bool _overwriteOnComplete = false);
- L82: void Stop();
- L91: void Debug();
- L94: Vector3 ToWorld(const Vector3& _local) const;
- L96: void LoadWorkList();
- L97: void LoadWork(const std::string& _key);
- L98: void OnComplete();
- L100: float   ApplyEasing(float _t, TimeEasing _easing) const;
- L101: Vector3 InterpolatePosition(const Keyframe& _a, const Keyframe& _b, float _t) const;
- L102: Vector2 InterpolateRotation(const Keyframe& _a, const Keyframe& _b, float _t, const Vector3& _worldPos) const;
- L103: Vector2 CalculateLookAtRotation(const Vector3& _position, const Vector3& _target) const;
- L105: PathType    StringToPathType(const std::string& _str)   const;
- L106: std::string PathTypeToString(PathType _type)             const;
- L107: TimeEasing  StringToTimeEasing(const std::string& _str) const;
- L108: std::string TimeEasingToString(TimeEasing _type)         const;
- L112: void SaveWork(const std::string& _key, const Work& _work);
- L113: void StartEditingWork(const std::string& _key);
- L114: void StopEditingWork();
- L115: void DeleteWork(const std::string& _key);
- L116: void AddKeyframe();
- L117: void RemoveKeyframe(int _index);
- L118: void CaptureCurrentCameraAsKeyframe();
- L119: void PreviewKeyframe(int _index);
- L120: void StopPreview();

### .\src\Camera\Repository\CameraRepository.hpp
- L16: void Initialize(float _ratio);
- L18: Camera* Add(const std::string& _name = "");
- L19: Camera* Get(const std::string& _name);
- L20: void Remove(const std::string& _name);
- L21: bool Contains(const std::string& _name) const;
- L22: bool IsEmpty() const;
- L24: std::vector<std::string> GetNames() const;
- L25: std::string GetFirstName() const;
- L27: void LoadFromFile();
- L28: void SaveToFile();
- L29: void Clear();
- L32: std::string GenerateUniqueName();

### .\src\Collision\CollisionManager.hpp
- L19: void Initialize(DebugUI* _debugUI);
- L20: void Update();
- L21: void DrawDebug() const;
- L22: void Debug();
- L25: void RebuildDebugLines();
- L26: void DrawSphere(const Vector3& center, float radius);
- L27: void DrawAABB(const Vector3& center, const Vector3& half);

### .\src\Common\Common.hpp
- L33: virtual void Initialize(DirectXAdapter* _adapter, DebugUI* _debugUi) = 0;
- L34: virtual void Update();
- L35: virtual void Debug();
- L36: virtual void Draw(Renderer* _renderer);
- L38: void RegisterDebug(const std::string& _id, const std::function<void()>& _func);
- L39: void RegisterUpdate(const std::string& _id, const std::function<void()>& _func);
- L40: void RegisterDraw(const std::function<void()>& _command, bool _isApplyPostEffect = true);
- L42: void Unregister(const std::string& _uuid);
- L48: void Setup(DirectXAdapter* _adapter, DebugUI* _debugUi, const std::string& _windowName);

### .\src\Debug\Debugger.hpp
- L34: Debugger();
- L35: ~Debugger();
- L58: WatchDebugger::Watch(_label, _ptr);
- L69: WatchDebugger::WatchEdit(_label, _ptr, _jsonFile, _jsonKey);
- L80: WatchDebugger::BeginGroup(_label);
- L85: WatchDebugger::BeginGroup(_label, _json, _name, _group);
- L96: WatchDebugger::Watch(_label, _ptr);
- L105: WatchDebugger::WatchEdit(_label, _ptr, _jsonFile, _jsonKey);

### .\src\Debug\FrameDebugger.hpp
- L24: void Initialize(DebugUI* _debugUi);

### .\src\DirectX\Compute\ComputePipeline.hpp
- L22: ComputePipeline& SetRootSignature(const RootSignature& _rootSignature);
- L23: void Create(const std::string& _name);

### .\src\DirectX\DirectXAdapter.hpp
- L84: DirectXAdapter(HWND _hWnd, size_t _width, size_t _height);
- L85: ~DirectXAdapter();
- L120: std::unique_ptr<DX12Resource> CreateUnorderedAccessView() const;
- L146: void EnableDebugLayer();
- L147: bool CreateDXGI();
- L149: bool CreateCommand();
- L150: bool CreateSwapChain();
- L151: bool CreateFence();
- L152: bool CreateRTV();
- L153: bool CreateDSV();
- L154: bool CreateViewportAndScissor();
- L155: bool CreateLimiter();
- L157: void SetSwapChainRenderTarget() const;
- L158: void Present();
- L159: void Wait();

### .\src\DirectX\FrameRate\FrameRateLimiter.hpp
- L14: explicit FrameRateLimiter(uint16_t _maxFps, bool _useVsync = true);
- L15: void WaitForNextFrame();
- L17: float GetCurrentFps() const;
- L18: float GetMaxFps() const;

### .\src\DirectX\GraphicsPipeline\Object\PipelineStateObject.hpp
- L36: PipelineStateObject(DirectXAdapter* _adapter);
- L37: PipelineStateObject& SetRootSignature(const RootSignature& _rootSignature);
- L38: PipelineStateObject& SetInputLayout(const InputLayout& _inputLayout);
- L39: PipelineStateObject& SetBlend(const D3D12_BLEND_DESC& _blendDesc);
- L40: PipelineStateObject& SetBlend(BlendMode _blendMode);
- L41: PipelineStateObject& SetRasterizer(const D3D12_RASTERIZER_DESC _rasterizer);
- L42: PipelineStateObject& SetDepthStencil(const D3D12_DEPTH_STENCIL_DESC& _depthStencilDesc);
- L43: PipelineStateObject& SetShader(std::unique_ptr<Shader> _shader);
- L44: PipelineStateObject& SetTopologyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE _topologyType);
- L45: PipelineStateObject& SetDSVFormat(DXGI_FORMAT _format);
- L47: void Create();
- L48: void DrawCall() const;

### .\src\DirectX\Heap\SRVManager.h
- L21: void Initialize(DirectXAdapter* _adapter);
- L22: void Finalize();
- L24: uint32_t Allocate();
- L25: void PreDraw() const;
- L27: void CreateSRVForTexture2D(uint32_t srvIndex, ID3D12Resource* pResource, DXGI_FORMAT format, UINT mipMap) const;
- L28: void CreateSRVForStructuredBuffer(uint32_t srvIndex, ID3D12Resource* pResource, UINT numElements, UINT stride) const;
- L29: void CreateSRVForCubeMap(uint32_t _srvIndex, ID3D12Resource* _pResource, DXGI_FORMAT _format) const;
- L31: void SetGraphicsRootDescriptorTable(UINT rootParameterIndex, uint32_t srvIndex) const;
- L37: ID3D12DescriptorHeap* GetDescriptorHeap() const;
- L39: D3D12_CPU_DESCRIPTOR_HANDLE GetCPUHandle(uint32_t index) const;
- L40: D3D12_GPU_DESCRIPTOR_HANDLE GetGPUHandle(uint32_t index) const;

### .\src\DirectX\LeakChecker\D3DResourceLeakChecker.hpp
- L6: ~D3DResourceLeakChecker();

### .\src\DirectX\RootSignature\InputLayout.hpp
- L11: InputLayout& SetElement(const D3D12_INPUT_ELEMENT_DESC& _element);

### .\src\DirectX\RootSignature\RootSignature.hpp
- L17: RootSignature& AddParameter(const D3D12_ROOT_PARAMETER& _parameter);
- L18: RootSignature& SetSampler(const D3D12_STATIC_SAMPLER_DESC& _sampler);
- L22: ID3D12RootSignature* Get() const;

### .\src\DirectX\Shader\Shader.h
- L19: Shader(const std::wstring& _name);
- L20: Shader(const std::wstring& _vs, const std::wstring& _ps);
- L23: Shader* PSLoad(const std::wstring& _name);
- L25: IDxcBlob* CompileCS(const std::wstring& _name) const;
- L35: bool Create(const std::wstring& _name);
- L36: bool Create(const std::wstring& _vs, const std::wstring& _ps);
- L37: void CreateDxc();
- L38: void CompileShaders();
- L39: void CompileShaders(const std::wstring& _vs, const std::wstring& _ps);
- L40: static std::wstring FindShaderDirectory();
- L41: static IDxcBlob* Compile(const std::wstring& directoryPath_, const std::wstring& _filePath, const wchar_t* _profile, IDxcUtils* _dxcUtils, IDxcCompiler3* _dxcCompiler, IDxcIncludeHandler* _includeHandler);

### .\src\Json\JsonParams.hpp
- L40: void SetValue(const std::string& _name, const std::string& _group, const std::string& _key, const Value& _value);
- L41: Group GetGroups(const std::string& _name);
- L42: Value GetValue(const std::string& _name, const std::string& _group, const std::string& _key) const;
- L43: void RemoveGroup(const std::string& _name, const std::string& _group);
- L45: bool Load(const std::string& _path, const std::string& _name = "");
- L46: void Save(const std::string& _path, std::string _name = "");
- L49: void Register(const std::string& _name);
- L50: void LoadJson(const std::string& _path, std::string _name);

### .\src\Light\DirectionalLight\DirectionalLight.h
- L23: void DefaultSetting() override;
- L24: void Save(std::string _path) override;
- L31: void ImGuiSetting(int _index) override;
- L34: void FollowRef() override;

### .\src\Light\LightManager.hpp
- L57: ~LightManager();
- L84: void ClearRef();
- L86: void Debug();
- L89: void CheckState();
- L90: void UpdateLights();
- L92: void Load();
- L93: void Save() const;

### .\src\Light\PointLight\PointLight.h
- L24: void DefaultSetting() override;
- L25: void Set(const std::string& uuid, const PointLight& pl);
- L26: void Save(std::string _path) override;
- L27: void ImGuiSetting(int _index) override;
- L30: void FollowRef() override;

### .\src\Light\RawLight.h
- L15: RawLight();
- L21: void Update();
- L23: bool IsEnable() const;
- L50: virtual void ImGuiSetting(int _index) = 0;
- L53: virtual void FollowRef() = 0;
- L63: FollowRef();

### .\src\Light\SpotLight\SpotLight.h
- L33: void DefaultSetting() override;
- L34: void Set(const std::string& uuid, const SpotLight& sl);
- L35: void Save(std::string _path) override;
- L36: void ImGuiSetting(int _index) override;
- L39: void FollowRef() override;

### .\src\Line\Common\LineCommon.hpp
- L11: void Initialize(DirectXAdapter* _adapter, DebugUI* _debugUi, SRVManager* _srv);
- L28: void Initialize(DirectXAdapter* _adapter, DebugUI* _debugUi) override;

### .\src\Mesh\Mesh.hpp
- L113: void EnableLighting(bool _active);

### .\src\Mesh\Repository\MeshRepository.hpp
- L15: void Initialize(DirectXAdapter* _adapter);
- L17: void Add(const std::string &_name, const MeshData& _raw);
- L18: MeshData Get(const std::string& _name);
- L19: bool Contains(const std::string& _name) const;

### .\src\Model\Common\ModelCommon.hpp
- L17: void Initialize(DirectXAdapter *_adapter, DebugUI *_debugUi) override;
- L18: void CreateSkinningPipeline() const;
- L19: void CreateStaticPipeline() const;
- L22: void Initialize(DirectXAdapter* _adapter, DebugUI* _debugUi, ResourceRepository* _resource, SRVManager* _srv);
- L24: void RegisterStaticDraw(const std::function<void()>& _command, bool _isApplyPostEffect = true);
- L25: void RegisterSkinningDraw(const std::function<void()>& _command, bool _isApplyPostEffect = true);
- L27: void Draw(Renderer* _renderer) override;
- L29: void DrawSkinning() const;
- L30: void DrawStatic() const;

### .\src\Model\Loader\GltfLoader.hpp
- L11: void LoadModel(const std::string& _name, ResourceRepository* _repository) override;
- L14: void LoadGltf(const std::string& _directory, const std::string& _name, const ResourceRepository* _repository) const;
- L16: static Node LoadNode(const aiNode* _node);
- L18: static std::optional<Animation> LoadAnimation(const aiScene* _scene, const std::string& _name);
- L20: static Skeleton CreateSkeleton(const Node& _root);
- L22: static int32_t CreateJoint(const Node& _node, const std::optional<int32_t>& _parent, std::vector<Joint>& _joints);
- L24: MeshData LoadMesh(const aiScene* _scene, const std::string& _name, ModelData& _model) const;
- L26: static void LoadVertexData(const aiMesh* _mesh, MeshData& _data);
- L27: static void LoadIndexData(const aiMesh* _mesh, MeshData& _data);
- L28: static void LoadBones(const aiMesh* _mesh, ModelData& _model);

### .\src\Model\Loader\IModelLoader.hpp
- L12: virtual void LoadModel(const std::string& _name, ResourceRepository* _repository) = 0;

### .\src\Model\Loader\ObjLoader.hpp
- L7: void LoadModel(const std::string& _name, ResourceRepository* _repository) override;

### .\src\Model\Repository\ModelRepository.hpp
- L27: bool Contains(const std::string& _name) const;

### .\src\ParticleSystem\Emitter\Emitter.hpp
- L76: Emitter(DirectXAdapter* _adapter, SRVManager* _srv);
- L77: void Initialize(const MeshData& _mesh);
- L78: void Update();
- L79: void Draw();
- L80: void Emit();
- L82: void Debug();
- L90: Emitter& Enable(bool _active = true);
- L92: Emitter& SetPosition(const Vector3& _position);
- L94: Emitter& SetFrequency(const float& _frequency);
- L96: Emitter& SetDuration(const float& _duration);
- L98: Emitter& SetSpawnCount(const uint16_t& _count);
- L100: Emitter& SetColor(const Vector4& _color);
- L102: Emitter& SetTexture(const std::string& _texture);
- L104: Emitter& SetSize(const float& _size);
- L106: Emitter& SetSize(const Vector3& _size);
- L108: Emitter& SetVelocity(const Vector3& _velocity);
- L121: void FrequencyUpdate();
- L122: void Spawn(const uint16_t& _count);
- L123: void RegisterGpu();

### .\src\ParticleSystem\Particle\Particle.hpp
- L22: void Initialize(float _duration);
- L23: void Update();
- L25: void Debug();
- L27: bool IsDead() const;
- L29: Vector3 GetPosition() const;
- L30: Vector3 GetScale() const;
- L31: Vector4 GetColor() const;
- L33: Particle& SetOrigin(const Vector3& _origin);
- L34: Particle& SetPosition(const Vector3& _position);
- L35: Particle& SetScale(const Vector3& _scale);
- L36: Particle& SetVelocity(const Vector3& _velocity);
- L37: Particle& SetColor(const Vector4& _color);

### .\src\ParticleSystem\ParticleSystem.hpp
- L44: explicit TemplateEditor(Template* _template);
- L45: TemplateEditor& AddEmitter(const EmitterConfig& _config);
- L69: ParticleSystem(DirectXAdapter* _adapter, SRVManager* _srv, MeshRepository* _mesh, DebugUI* _debugUI);
- L71: void Initialize();
- L72: void Update();
- L73: void Draw(Renderer* _renderer);
- L126: void Debug();
- L129: void SetupPSO();
- L130: void InitializePool();
- L131: void LoadTemplates();
- L132: UpdateFunc ResolveUpdateFunc(const EmitterConfig& _config) const;
- L133: SpawnFunc  ResolveSpawnFunc(const EmitterConfig& _config) const;

### .\src\Platform\WinApp.hpp
- L13: void Initialize();
- L15: bool IsEnabled() const ;
- L17: HWND GetWindowHandle() const;
- L19: HINSTANCE GetInstanceHandle() const;
- L21: void SetWindowSize(int _width, int _height) const;
- L23: void SetTitle(const std::string& _title) const;
- L25: void ToggleBorderless() const;
- L27: void GetClientSize(int& _width, int& _height) const;

### .\src\PostProcess\BoxBlur\BoxBlur.hpp
- L14: void Initialize() override;
- L15: void Debug() override;
- L18: void Modifier() override;
- L21: void LoadPreset(const std::string& _presetName) override;
- L22: void SavePreset(const std::string& _presetName) override;
- L23: nlohmann::json SaveParameters() const override;
- L24: void UpdateAnimation(float _t) override;

### .\src\PostProcess\Editor\PostProcessPresetEditor.hpp
- L166: PresetInfo GetPresetInfo(const std::string& _presetName) const;
- L174: void RenderCreateNewPresetSection();
- L175: void RenderPresetConfigurationSection();
- L176: void RenderBasicSettings();
- L177: void RenderMembersList();
- L178: void RenderIgnoreList();
- L179: void RenderAddMemberDialog();
- L180: void RenderQuickSaveSection();
- L184: void RenderPointsList();
- L191: void AddKeyframePoint(const std::string& _pointName);
- L192: void RemoveKeyframePoint(int _pointIndex);
- L193: void MovePointUp(int _pointIndex);
- L194: void MovePointDown(int _pointIndex);

### .\src\PostProcess\Executor\PostProcessExecutor.hpp
- L97: void BeginFrame() const;
- L98: void EndFrame() const;
- L99: void Execute();
- L100: void Draw() const;
- L102: void SetActive(const std::string& _name, bool _enable);
- L104: void Debug();
- L154: void CreateSceneRenderTexture();

### .\src\PostProcess\Grayscale\Grayscale.hpp
- L17: void Initialize() override;
- L18: void Debug() override;
- L21: void Modifier() override;
- L24: void LoadPreset(const std::string& _presetName) override;
- L25: void SavePreset(const std::string& _presetName) override;
- L26: nlohmann::json SaveParameters() const override;
- L27: void UpdateAnimation(float _t) override;

### .\src\PostProcess\IPostEffect.hpp
- L133: void CreateOutput();
- L134: virtual void Modifier() = 0;

### .\src\PostProcess\Vignette\Vignette.hpp
- L30: void Initialize() override;
- L31: void Debug() override;
- L35: void SavePreset(const std::string& _presetName) override;
- L36: nlohmann::json SaveParameters() const override;
- L37: void UpdateAnimation(float _t) override;
- L40: void Modifier() override;

### .\src\Scene\SceneSwitcher.hpp
- L42: SceneSwitcher();
- L83: const Context& GetContext() const;

### .\src\Scene\Transition\Fade.hpp
- L19: Fade();
- L22: void Initialize() override;
- L23: void Update() override;
- L24: void Draw() override;
- L26: void Start(State _state, float _duration) override;
- L27: void Stop() override;
- L29: bool IsFinished() const override;

### .\src\Scene\Transition\Transition.hpp
- L27: void Initialize();
- L28: void Update();
- L29: void Draw();
- L31: void Awake(Type _type, ITransitionEffect::State _state);
- L32: void Awake(Type _type, ITransitionEffect::State _state, float _duration);
- L34: bool InProgress();
- L36: void SetDefaultDuration(float _duration);
- L43: ITransitionEffect::State GetCurrentState() const;
- L44: float GetProgress() const;
- L47: void CreateEffect(Type _type);

### .\src\Scheduler\Scheduler.hpp
- L12: Scheduler();
- L13: ~Scheduler();
- L15: void RunTaskLater(Task _task, std::chrono::milliseconds _delay);
- L16: void RunTaskTimer(Task _task, std::chrono::milliseconds _interval);
- L31: void Work();
- L32: void Shutdown();

### .\src\Sky\Common\SkyCommon.hpp
- L8: void Initialize(DirectXAdapter* _adapter, DebugUI* _debugUi) override;

### .\src\Sky\Skybox.hpp
- L49: void Initialize(const std::string& _texture);
- L50: void Update();
- L51: void Draw();
- L53: void SetColor(const Vector4& _color) const;
- L56: void CreateVertex();
- L57: void CreateIndex();

### .\src\Sprite\Common\SpriteCommon.hpp
- L7: void Initialize(DirectXAdapter *_adapter, DebugUI *_debugUi) override;

### .\src\Stage\LevelEditor.hpp
- L27: LevelEditor(DebugUI* _debug);
- L28: void Initialize(const std::string& _name);
- L29: void Update();
- L30: void Draw() const;
- L33: void Spawn();
- L34: void Debug();

### .\src\Stage\Loader\StageLoader.hpp
- L15: void Initialize(StageRepository* _repository);
- L16: bool Load(const std::string& _path) const;
- L18: static std::unique_ptr<LevelData> Recursive(const nlohmann::json& _base);

### .\src\Texture\TextureManager.hpp
- L46: ~TextureManager();
- L94: DirectX::ScratchImage LoadTexture(const std::string& _filename) const;
- L95: void UploadTextureData(DX12Resource* _texture, const DirectX::ScratchImage& _mipImages) const;
- L97: static DirectX::ScratchImage LoadDDS(const std::wstring& _path);

### .\src\Timer\Timer.hpp
- L12: Timer(std::chrono::milliseconds _duration);
- L13: void Start();
- L14: void Stop();
- L15: void Reset();
- L16: void Restart();
- L18: bool Check();
- L20: void SetDuration(std::chrono::milliseconds _duration);

### .\src\Window\Window.hpp
- L68: static LRESULT CALLBACK WindowProc(HWND _hwnd, UINT _uMsg, WPARAM _wParam, LPARAM _lParam);

## Summary

- `///` violations: 60
- Declarations with potentially missing comments: 378

