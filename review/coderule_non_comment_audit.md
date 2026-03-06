# Coderule Non-Comment Audit

- Scope: include/**, src/**
- Focus: non-comment rule violations + future refactor targets
- Priority policy: writing issues (typo/inconsistent spelling) first

## P0 (Highest): Writing Violations (Typos / Inconsistent Spelling)

- .\src\DirectX\DirectXAdapter.hpp:239 | guard=`DirectXAdapter_HPP_` | note=`DirectXAdaptor_HPP_`
- .\src\Mesh\Data\MeshData.hpp:22 | guard=`MESH_DATA_HPP_` | note=`MeshData_HPP_`
- .\src\Mesh\Repository\MeshRepository.hpp:22 | guard=`MESH_REPOSITORY_HPP_` | note=`MeshLoader_HPP_`
- .\src\Model\Loader\ObjLoader.hpp:1 | guard=`ModelLoader_HPP_` | note=`guard should likely be OBJLOADER_HPP_`

## P1: Confirmed Coderule Violations (Non-Comment)

### 1) Header extension rule (.hpp required, .h disallowed)
- Violations: 6
- .\src\DirectX\Heap\SRVManager.h
- .\src\DirectX\Shader\Shader.h
- .\src\Light\DirectionalLight\DirectionalLight.h
- .\src\Light\PointLight\PointLight.h
- .\src\Light\RawLight.h
- .\src\Light\SpotLight\SpotLight.h

### 2) `#pragma once` usage (guard style required by Coderule)
- Violations: 7
- .\src\Collision\CollisionManager.hpp:1
- .\src\DirectX\Heap\SRVManager.h:1
- .\src\DirectX\Shader\Shader.h:1
- .\src\Light\DirectionalLight\DirectionalLight.h:1
- .\src\Light\PointLight\PointLight.h:1
- .\src\Light\RawLight.h:1
- .\src\Light\SpotLight\SpotLight.h:1

### 3) Include guard naming format (`[FILE_NAME]_HPP_`, ALL_UPPER)
- Violations: 74
- Examples (first 25):
- .\include\DebugUI.hpp:1 | guard=`DebugUI_HPP_` | expected=`DEBUGUI_HPP_` | issue=not ALL_UPPER
- .\include\Factory\AbstractPostEffectFactory.hpp:1 | guard=`AbstractPostEffectFactory_HPP_` | expected=`ABSTRACTPOSTEFFECTFACTORY_HPP_` | issue=not ALL_UPPER
- .\include\Factory\AbstractSceneFactory.hpp:1 | guard=`AbstractSceneFactory_HPP_` | expected=`ABSTRACTSCENEFACTORY_HPP_` | issue=not ALL_UPPER
- .\include\Framework.hpp:1 | guard=`Framework_HPP_` | expected=`FRAMEWORK_HPP_` | issue=not ALL_UPPER
- .\include\IGame.hpp:1 | guard=`IGame_HPP_` | expected=`IGAME_HPP_` | issue=not ALL_UPPER
- .\include\Input.hpp:1 | guard=`Input_HPP_` | expected=`INPUT_HPP_` | issue=not ALL_UPPER
- .\include\IScene.hpp:1 | guard=`IScene_HPP_` | expected=`ISCENE_HPP_` | issue=not ALL_UPPER
- .\include\Line.hpp:1 | guard=`Line_HPP_` | expected=`LINE_HPP_` | issue=not ALL_UPPER
- .\include\Log.hpp:1 | guard=`LOG_HPP` | expected=`LOG_HPP_` | issue=missing _HPP_ suffix, does not match file name
- .\include\Model.hpp:1 | guard=`Model_HPP_` | expected=`MODEL_HPP_` | issue=not ALL_UPPER
- .\include\PerformanceProfiler.hpp:1 | guard=`PerformanceProfiler_HPP_` | expected=`PERFORMANCEPROFILER_HPP_` | issue=not ALL_UPPER
- .\include\Sprite.hpp:1 | guard=`Sprite_HPP_` | expected=`SPRITE_HPP_` | issue=not ALL_UPPER
- .\include\WatchDebugger.hpp:1 | guard=`WatchDebugger_HPP_` | expected=`WATCHDEBUGGER_HPP_` | issue=not ALL_UPPER
- .\src\Animation\Animation.hpp:1 | guard=`Animation_HPP_` | expected=`ANIMATION_HPP_` | issue=not ALL_UPPER
- .\src\Animation\KeyFrame.hpp:1 | guard=`KeyFrame_HPP_` | expected=`KEYFRAME_HPP_` | issue=not ALL_UPPER
- .\src\Animation\NodeAnimation.hpp:1 | guard=`NodeAnimation_HPP_` | expected=`NODEANIMATION_HPP_` | issue=not ALL_UPPER
- .\src\Camera\Camera.hpp:1 | guard=`Camera_HPP_` | expected=`CAMERA_HPP_` | issue=not ALL_UPPER
- .\src\Camera\Controller\CameraController.hpp:1 | guard=`CameraController_HPP_` | expected=`CAMERACONTROLLER_HPP_` | issue=not ALL_UPPER
- .\src\Camera\Director\CameraDirector.hpp:1 | guard=`CameraDirector_HPP_` | expected=`CAMERADIRECTOR_HPP_` | issue=not ALL_UPPER
- .\src\Camera\Repository\CameraRepository.hpp:1 | guard=`CameraRepository_HPP_` | expected=`CAMERAREPOSITORY_HPP_` | issue=not ALL_UPPER
- .\src\Common\Common.hpp:1 | guard=`Common_HPP_` | expected=`COMMON_HPP_` | issue=not ALL_UPPER
- .\src\Config\Config.hpp:1 | guard=`Config_HPP_` | expected=`CONFIG_HPP_` | issue=not ALL_UPPER
- .\src\Container\Material.hpp:1 | guard=`Material_HPP_` | expected=`MATERIAL_HPP_` | issue=not ALL_UPPER
- .\src\Debug\Debugger.hpp:1 | guard=`Debugger_HPP_` | expected=`DEBUGGER_HPP_` | issue=not ALL_UPPER
- .\src\Debug\FrameDebugger.hpp:1 | guard=`FrameDebugger_HPP_` | expected=`FRAMEDEBUGGER_HPP_` | issue=not ALL_UPPER

## P2: Refactor Targets (Future Work)

Large files are likely carrying multiple responsibilities and should be split by feature/module boundaries.
- .\src\PostProcess\Editor\PostProcessPresetEditor.cpp | 1172 lines
- .\src\Camera\Director\CameraDirector.cpp | 635 lines
- .\src\DirectX\DirectXAdapter.cpp | 616 lines
- .\src\PostProcess\Executor\PostProcessExecutor.cpp | 504 lines
- .\src\Model\Model.cpp | 414 lines
- .\src\ParticleSystem\ParticleSystem.cpp | 410 lines
- .\src\Model\Common\ModelCommon.cpp | 369 lines
- .\src\Debug\DebugUI.cpp | 355 lines
- .\src\System\Log.cpp | 334 lines
- .\src\Light\LightManager.cpp | 296 lines
- .\src\Debug\WatchDebugger.cpp | 250 lines
- .\src\Texture\TextureManager.cpp | 205 lines

## Notes
- This report intentionally excludes comment-style violations (already reported separately).
- Some guard naming checks are strict by design, aligned to `[FILE_NAME]_HPP_`.

