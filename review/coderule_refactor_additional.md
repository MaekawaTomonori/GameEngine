# Additional Refactor Candidates (Excluding externals)

- Scope: `src/**`, `include/**`
- Excluded: `externals/**`
- Priority rule: writing issues remain highest (see previous report). This addendum focuses on function/class split and redundancy.

## P1: Function Split Candidates (Long Methods)
- Heuristic: method body length >= 80 lines

- .\src\Camera\Director\CameraDirector.cpp:481 (170 lines) `void CameraDirector::ShowEditor() {`
- .\src\PostProcess\Editor\PostProcessPresetEditor.cpp:58 (152 lines) `void PostProcessPresetEditor::RenderAvailablePresetsSection() {`
- .\src\PostProcess\Editor\PostProcessPresetEditor.cpp:304 (122 lines) `void PostProcessPresetEditor::RenderMembersList() {`
- .\src\ParticleSystem\ParticleSystem.cpp:271 (121 lines) `void ParticleSystem::Debug() {`
- .\src\System\Log.cpp:280 (120 lines) `void Log::Debug(DebugUI* _debug) {`
- .\src\PostProcess\Editor\PostProcessPresetEditor.cpp:1248 (115 lines) `void PostProcessPresetEditor::RenderPointParameters() {`
- .\src\PostProcess\Executor\PostProcessExecutor.cpp:400 (112 lines) `void PostProcessExecutor::ApplyPreset(const std::string& _presetName, const std::string& _mode, const std::vector<std::string>& _ignoreList, std::function<void()> _onComplete) {`
- .\src\PostProcess\Editor\PostProcessPresetEditor.cpp:1005 (110 lines) `void PostProcessPresetEditor::RenderPointsList() {`
- .\src\Camera\Director\CameraDirector.cpp:371 (109 lines) `void CameraDirector::Debug() {`
- .\src\Scene\SceneSwitcher.cpp:92 (100 lines) `void SceneSwitcher::Debug() {`
- .\src\Json\JsonParams.cpp:13 (90 lines) `void JsonParams::LoadJson(const std::string& _path, std::string _name) {`
- .\src\PostProcess\Executor\PostProcessExecutor.cpp:174 (89 lines) `void PostProcessExecutor::Debug() {`
- .\src\Debug\DebugUI.cpp:28 (82 lines) `void DebugUI::Initialize(const DirectXAdapter *_adapter) {`
- .\src\Debug\DebugUI.cpp:326 (80 lines) `void DebugUI::SetupStyle() {`

## P1: Class Extraction Candidates (Responsibility Concentration)
- Heuristic: many medium/large methods (>= 40 lines each) in one class

- Class `PostProcessPresetEditor` in .\src\PostProcess\Editor\PostProcessPresetEditor.cpp | methods=10, total=811, max=152
- Class `PostProcessExecutor` in .\src\PostProcess\Executor\PostProcessExecutor.cpp | methods=6, total=400, max=112
- Class `CameraDirector` in .\src\Camera\Director\CameraDirector.cpp | methods=3, total=347, max=170
- Class `DebugUI` in .\src\Debug\DebugUI.cpp | methods=4, total=280, max=82
- Class `ParticleSystem` in .\src\ParticleSystem\ParticleSystem.cpp | methods=3, total=242, max=121
- Class `Model` in .\src\Model\Model.cpp | methods=3, total=190, max=73
- Class `DirectXAdapter` in .\src\DirectX\DirectXAdapter.cpp | methods=3, total=182, max=77
- Class `LightManager` in .\src\Light\LightManager.cpp | methods=3, total=167, max=78
- Class `JsonParams` in .\src\Json\JsonParams.cpp | methods=2, total=159, max=90
- Class `SceneSwitcher` in .\src\Scene\SceneSwitcher.cpp | methods=2, total=152, max=100

Suggested split direction:
- `PostProcessPresetEditor`: split into list UI / editor form / keyframe panel / serialization service.
- `CameraDirector`: split runtime interpolation engine vs debug/editor UI.
- `PostProcessExecutor`: split preset transition scheduler vs pass execution pipeline.
- `DebugUI`: split docking/menu management vs widget/command registry.

## P2: Redundancy / Duplication Candidates
- Heuristic: identical 5-line blocks appearing in 6+ locations

### Cluster 1 (occurrences=10)
- .\src\Line\Common\LineCommon.cpp:18
- .\src\Line\Common\LineCommon.cpp:26
- .\src\Model\Common\ModelCommon.cpp:56
- .\src\Model\Common\ModelCommon.cpp:64
- .\src\Model\Common\ModelCommon.cpp:194
- .\src\Model\Common\ModelCommon.cpp:202
- .\src\Sky\Common\SkyCommon.cpp:26
- .\src\Sky\Common\SkyCommon.cpp:34
- Action: extract common helper/base routine for shared setup or parameter application logic.

### Cluster 2 (occurrences=9)
- .\src\Model\Common\ModelCommon.cpp:30
- .\src\Model\Common\ModelCommon.cpp:46
- .\src\Model\Common\ModelCommon.cpp:176
- .\src\PostProcess\BoxBlur\BoxBlur.cpp:8
- .\src\PostProcess\Executor\PostProcessExecutor.cpp:46
- .\src\PostProcess\Grayscale\Grayscale.cpp:8
- .\src\PostProcess\Vignette\Vignette.cpp:16
- .\src\Sky\Common\SkyCommon.cpp:16
- Action: extract common helper/base routine for shared setup or parameter application logic.

### Cluster 3 (occurrences=9)
- .\src\Model\Common\ModelCommon.cpp:141
- .\src\Model\Common\ModelCommon.cpp:270
- .\src\ParticleSystem\ParticleSystem.cpp:423
- .\src\PostProcess\BoxBlur\BoxBlur.cpp:35
- .\src\PostProcess\Executor\PostProcessExecutor.cpp:65
- .\src\PostProcess\Grayscale\Grayscale.cpp:35
- .\src\PostProcess\Vignette\Vignette.cpp:43
- .\src\Sky\Common\SkyCommon.cpp:53
- Action: extract common helper/base routine for shared setup or parameter application logic.

### Cluster 4 (occurrences=9)
- .\src\Model\Common\ModelCommon.cpp:140
- .\src\Model\Common\ModelCommon.cpp:269
- .\src\ParticleSystem\ParticleSystem.cpp:422
- .\src\PostProcess\BoxBlur\BoxBlur.cpp:34
- .\src\PostProcess\Executor\PostProcessExecutor.cpp:64
- .\src\PostProcess\Grayscale\Grayscale.cpp:34
- .\src\PostProcess\Vignette\Vignette.cpp:42
- .\src\Sky\Common\SkyCommon.cpp:52
- Action: extract common helper/base routine for shared setup or parameter application logic.

### Cluster 5 (occurrences=9)
- .\src\Model\Common\ModelCommon.cpp:142
- .\src\Model\Common\ModelCommon.cpp:271
- .\src\ParticleSystem\ParticleSystem.cpp:424
- .\src\PostProcess\BoxBlur\BoxBlur.cpp:36
- .\src\PostProcess\Executor\PostProcessExecutor.cpp:66
- .\src\PostProcess\Grayscale\Grayscale.cpp:36
- .\src\PostProcess\Vignette\Vignette.cpp:44
- .\src\Sky\Common\SkyCommon.cpp:54
- Action: extract common helper/base routine for shared setup or parameter application logic.

### Cluster 6 (occurrences=9)
- .\src\Model\Common\ModelCommon.cpp:31
- .\src\Model\Common\ModelCommon.cpp:47
- .\src\Model\Common\ModelCommon.cpp:177
- .\src\PostProcess\BoxBlur\BoxBlur.cpp:9
- .\src\PostProcess\Executor\PostProcessExecutor.cpp:47
- .\src\PostProcess\Grayscale\Grayscale.cpp:9
- .\src\PostProcess\Vignette\Vignette.cpp:17
- .\src\Sky\Common\SkyCommon.cpp:17
- Action: extract common helper/base routine for shared setup or parameter application logic.

## P2: Additional Cleanup Candidates
- `src/System/Log.cpp:280` (`Log::Debug`) mixes UI rendering and transport/config logic; split view-model from backend state.
- `src/Json/JsonParams.cpp:13` and `:172` (`LoadJson`/`Save`) are long and likely symmetric; centralize key traversal and error handling helpers.
- `src/Scene/SceneSwitcher.cpp:92` (`Debug`) is large for debug-only flow; move tab/panel fragments into small private render methods.

## P1: Naming Clarity Candidates (Public APIs / Class Names)
- Focus: names that are hard to understand without reading implementation, or names whose meaning does not match behavior.

- `src/Common/Common.hpp:10` class `Common`
  Current concern: too generic; role is engine render-system base with update/debug/draw registries.
  Suggested direction: `RenderSubsystemBase` or `EngineModuleBase`.

- `src/Platform/WinApp.hpp:7` class `WinApp`
  Current concern: overlaps conceptually with `Window` (`src/Window/Window.hpp`), boundary is unclear from names alone.
  Suggested direction: rename to `PlatformWindowHost` (wrapper) or merge naming with `Window`.

- `include/IScene.hpp:98` `PostEffect() const`
  Current concern: noun-only getter name is ambiguous (single effect vs manager/executor).
  Suggested direction: `GetPostProcessExecutor()` or `GetPostEffectExecutor()`.

- `include/IScene.hpp:99` `Particle() const`
  Current concern: noun-only getter; could be one particle object or system.
  Suggested direction: `GetParticleSystem()`.

- `include/Framework.hpp:97` `bool Check() const`
  Current concern: `Check` does not indicate what is checked (window state, resources, runtime health).
  Suggested direction: `IsSystemReady()`, `ValidateRuntimeState()`, or `CanContinueMainLoop()`.

- `src/Timer/Timer.hpp:18` `bool Check()`
  Current concern: unclear whether it means elapsed, timeout, running state, or one-shot trigger.
  Suggested direction: `HasElapsed()`, `IsExpired()`, or `TryConsumeElapsed()`.

- `src/PostProcess/IPostEffect.hpp:33` `SetUp(DirectXAdapter*, SRVManager*)`
  Current concern: inconsistent casing with common `Setup` spelling; easy to mistype with `SetUp` vs `Setup`.
  Suggested direction: `Setup(...)` (or `InitializeContext(...)` if semantics are broader).

- `src/PostProcess/IPostEffect.hpp:79` `virtual void Modifier() = 0;`
  Current concern: `Modifier` is noun-like and does not describe action.
  Suggested direction: `ApplyParameters()`, `UpdateConstants()`, or `RenderPass()`.

- `src/Scheduler/Scheduler.hpp:16` `RunTaskTimer(Task, milliseconds)`
  Current concern: phrase is unnatural; periodic scheduling semantics are not explicit.
  Suggested direction: `ScheduleRecurring(...)` or `RunTaskEvery(...)`.

- `src/Camera/Director/CameraDirector.hpp:80` `Load(const std::string& _key)`
  Current concern: too broad in a class that has `LoadWorkList/LoadWork`; target scope is unclear.
  Suggested direction: `LoadWorkByKey(...)` or `LoadCameraWork(...)`.

- `src/Camera/Director/CameraDirector.hpp:81` `Run(const std::string& _key, ...)`
  Current concern: `Run` is too generic for camera sequence playback.
  Suggested direction: `Play(...)`, `StartWork(...)`, or `PlayWork(...)`.

- `src/Scene/SceneSwitcher.hpp:64` `Change(const std::string& _name)`
  Current concern: operation is scene transition/switch, but `Change` is generic.
  Suggested direction: `ChangeScene(...)` or `SwitchToScene(...)`.

- `src/Light/LightManager.hpp:77` `Add(LightType _type)`
  Current concern: ambiguous object lifecycle (create + register? default initialize?).
  Suggested direction: `AddLight(...)`, `CreateLight(...)`, or `AddLightOfType(...)`.

- `src/Light/PointLight/PointLight.h:25`, `src/Light/SpotLight/SpotLight.h:34`, `src/Light/DirectionalLight/DirectionalLight.h:26` `Set(...)`
  Current concern: `Set` is too generic and argument semantics differ across classes.
  Suggested direction: `SetFromData(...)`, `ApplySerializedLight(...)`, or type-specific names such as `SetPointLight(...)`.

- `src/Stage/Loader/StageLoader.hpp:18` `static std::unique_ptr<LevelData> Recursive(const nlohmann::json& _base)`
  Current concern: name describes implementation style, not domain meaning.
  Suggested direction: `ParseLevelDataRecursive(...)` or `BuildLevelDataTree(...)`.

