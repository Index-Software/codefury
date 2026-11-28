cmake_minimum_required(VERSION 3.10...3.30)

project(codefury CXX)

enable_language(C)

set(CMAKE_BUILD_TYPE Release)
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0 -g")
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O0 -g")
elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2")
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O2")
endif()

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/../obj/linux-x86_64)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/../obj/linux-x86_64)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/../bin/linux-x86_64)
set(CMAKE_CXX_STANDARD 98)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -frtti")
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(ENGINE_SRCS
	src/engine/scene/Scene.cpp
	src/engine/renderer/RenderQueue.cpp
	src/engine/config/ConfigVar.cpp
	src/engine/ecs/EntityRegistry.cpp
	src/engine/ecs/Entity.cpp
	src/engine/ecs/components/Mesh.cpp
	src/engine/ecs/components/Camera.cpp
	src/engine/input/InputManager.cpp
	src/engine/input/InputActions.cpp
	src/engine/input/controllers/FreeCamera.cpp
	src/engine/audio/AudioManager.cpp
	src/engine/audio/AudioMixer.cpp
	src/engine/audio/Vorbis.cpp
	src/engine/audio/STBVorbis.c
	src/engine/renderer/material/Material.cpp
	src/engine/renderer/texture/ColorSpace.cpp
	src/engine/renderer/texture/Texture.cpp
	src/engine/renderer/ibl/Cubemap.cpp
	src/engine/renderer/ibl/EnvironmentFaces.cpp
	src/engine/renderer/ibl/CubemapUtils.cpp
	src/engine/renderer/ibl/CubemapIBL.cpp
	src/engine/renderer/ibl/CubemapSH.cpp
	src/engine/renderer/ibl/EnvironmentPrefilter.cpp
	src/engine/renderer/ibl/EnvironmentSerializer.cpp
	src/engine/renderer/ibl/EnvironmentLighting.cpp
	src/engine/renderer/RenderItem.cpp
	src/engine/renderer/ShadowResources.cpp
	src/engine/renderer/shader/Shader.cpp
	src/engine/renderer/Skybox.cpp
	src/engine/renderer/MeshBuilder.cpp
	src/engine/renderer/passes/MainLightShadowPass.cpp
	src/engine/renderer/passes/PointLightShadowPass.cpp
	src/engine/renderer/passes/ForwardBasePass.cpp
	src/engine/renderer/passes/ForwardAddPass.cpp
	src/engine/renderer/ui/UI.cpp
	src/engine/renderer/ui/UIBatch.cpp
	src/engine/renderer/BatchPool.cpp
	src/engine/renderer/ConstantBuffers.cpp
	src/engine/renderer/text/Font.cpp
	src/engine/renderer/text/TextRenderer.cpp
	thirdparty/gcmgl/src/utils/Utils.cpp
	src/engine/utils/RadixSort.cpp
	src/engine/utils/FileUtils.cpp
	src/engine/utils/StringUtils.cpp
	src/engine/utils/UniqueID.cpp
	src/engine/utils/KeyValueLine.cpp
	thirdparty/gcmgl/src/utils/Time.cpp
	src/engine/debug/Debug.cpp
	#src/engine/io/cfm/CFMLoader.cpp
	thirdparty/mathsfury/Maths.cpp
	thirdparty/mathsfury/Vector2.cpp
	thirdparty/mathsfury/Vector3.cpp
	thirdparty/mathsfury/Vector4.cpp
	thirdparty/mathsfury/Matrix4.cpp
	thirdparty/mathsfury/Quaternion.cpp
	thirdparty/gcmgl/src/renderer/Renderer.cpp
	thirdparty/gcmgl/src/renderer/BatchRenderer.cpp
	thirdparty/gcmgl/src/window/WindowManager.cpp
	thirdparty/gcmgl/src/utils/Color.cpp
	thirdparty/wren/src/vm/wren_vm.c
	thirdparty/wren/src/vm/wren_compiler.c
	thirdparty/wren/src/vm/wren_core.c
	thirdparty/wren/src/vm/wren_primitive.c
	thirdparty/wren/src/vm/wren_utils.c
	thirdparty/wren/src/vm/wren_value.c
	thirdparty/wren/src/vm/wren_debug.c
)

set(WREN_BIND_GEN ${CMAKE_CURRENT_SOURCE_DIR}/src/engine/wren/wrenbind/generated/WrenBindGenerated.cpp)
set(WREN_BIND_GEN_H ${CMAKE_CURRENT_SOURCE_DIR}/src/engine/wren/wrenbind/generated/WrenBindGenerated.h)

file(GLOB_RECURSE WREN_SCRIPT_INPUTS
	${CMAKE_SOURCE_DIR}/content/*.wren)

set(WREN_BIND_INPUTS
	${CMAKE_SOURCE_DIR}/src/engine/renderer/ui/UI.h
	${CMAKE_SOURCE_DIR}/src/engine/debug/Debug.h
	${CMAKE_SOURCE_DIR}/src/engine/scene/Scene.h
	${CMAKE_SOURCE_DIR}/src/engine/renderer/RenderQueue.h
	${CMAKE_SOURCE_DIR}/src/engine/input/Input.h
	${CMAKE_SOURCE_DIR}/src/engine/input/InputActions.h
	${CMAKE_SOURCE_DIR}/src/engine/audio/Audio.h
	${CMAKE_SOURCE_DIR}/src/engine/input/controllers/FreeCamera.h
	${CMAKE_SOURCE_DIR}/src/engine/renderer/Skybox.h
	${CMAKE_SOURCE_DIR}/src/engine/renderer/shader/Shader.h
	${CMAKE_SOURCE_DIR}/src/engine/renderer/material/Material.h
	${CMAKE_SOURCE_DIR}/src/engine/renderer/texture/Texture.h
	${CMAKE_SOURCE_DIR}/src/engine/ecs/components/Camera.h
	${CMAKE_SOURCE_DIR}/src/engine/ecs/EntityRegistry.h
	${CMAKE_SOURCE_DIR}/src/engine/ecs/components/Transform.h
	${CMAKE_SOURCE_DIR}/src/engine/ecs/components/Light.h
	${CMAKE_SOURCE_DIR}/src/engine/ecs/components/Mesh.h
	${CMAKE_SOURCE_DIR}/src/engine/config/ConfigVar.h
	${CMAKE_SOURCE_DIR}/src/engine/renderer/MeshBuilder.h
	${CMAKE_SOURCE_DIR}/src/engine/wren/bindings/CColor.h
	${CMAKE_SOURCE_DIR}/src/engine/wren/bindings/CVector2.h
	${CMAKE_SOURCE_DIR}/src/engine/wren/bindings/CVector3.h
	${CMAKE_SOURCE_DIR}/src/engine/wren/bindings/CMatrix4.h
	${CMAKE_SOURCE_DIR}/src/engine/wren/bindings/CMaths.h
	${CMAKE_SOURCE_DIR}/src/engine/wren/bindings/IRenderer.h
	${CMAKE_SOURCE_DIR}/src/engine/wren/bindings/CTime.h
	${CMAKE_SOURCE_DIR}/src/engine/wren/bindings/CFileUtils.h
	${CMAKE_SOURCE_DIR}/src/engine/display/CDisplay.h
	${CMAKE_SOURCE_DIR}/src/engine/application/Application.h
	${CMAKE_SOURCE_DIR}/src/engine/wren/bindings/Platform.h
	${CMAKE_SOURCE_DIR}/src/engine/wren/bindings/InputMods.h
	${CMAKE_SOURCE_DIR}/src/engine/wren/bindings/Anchor.h
	${CMAKE_SOURCE_DIR}/src/engine/wren/bindings/ShaderType.h
	${CMAKE_SOURCE_DIR}/src/engine/wren/bindings/LightType.h
	${CMAKE_SOURCE_DIR}/src/engine/wren/bindings/FaceOrientation.h
	${CMAKE_SOURCE_DIR}/src/engine/wren/bindings/TextureWrapMode.h
)

set(WREN_CODEGEN_INPUT_ARGS "")
foreach(_wrenInput ${WREN_BIND_INPUTS})
	list(APPEND WREN_CODEGEN_INPUT_ARGS --input "${_wrenInput}")
endforeach()

set(WREN_CODEGEN ${CMAKE_SOURCE_DIR}/thirdparty/wrenbind/wrenbind/generate_bindings.py)
set(WREN_CODEGEN_DEPS
	${WREN_CODEGEN}
	${CMAKE_SOURCE_DIR}/thirdparty/wrenbind/wrenbind/binding_generator.py
	${CMAKE_SOURCE_DIR}/thirdparty/wrenbind/wrenbind/bindtypes.py)

add_custom_command(
	OUTPUT ${WREN_BIND_GEN}
	OUTPUT ${WREN_BIND_GEN_H}
	COMMAND python3 ${WREN_CODEGEN}
		--wren-scripts ${CMAKE_SOURCE_DIR}/content
		${WREN_CODEGEN_INPUT_ARGS}
		--include ${CMAKE_SOURCE_DIR}
		--include ${CMAKE_SOURCE_DIR}/public
		--include ${CMAKE_SOURCE_DIR}/src
		--include ${CMAKE_SOURCE_DIR}/src/engine
		--include ${CMAKE_SOURCE_DIR}/thirdparty
		--include ${CMAKE_SOURCE_DIR}/thirdparty/mathsfury
		--include ${CMAKE_SOURCE_DIR}/thirdparty/gcmgl/src
		--include ${CMAKE_SOURCE_DIR}/thirdparty/gcmgl/public
		--include ${CMAKE_SOURCE_DIR}/thirdparty/simde
		--include ${CMAKE_SOURCE_DIR}/thirdparty/wren/src/include
		--include ${CMAKE_SOURCE_DIR}/thirdparty/wrenbind/include
		--include ${CMAKE_SOURCE_DIR}/thirdparty/wrenbind/public
		--force-include wren/wrenbind/WrenBindArg.h
		--force-include wren/wrenbind/WrenBindAlloc.h
		--output ${WREN_BIND_GEN}
		--header-output ${WREN_BIND_GEN_H}
	DEPENDS
		${WREN_CODEGEN_DEPS}
		${WREN_BIND_INPUTS}
		${WREN_SCRIPT_INPUTS}
	COMMENT "Generating Wren bindings..."
)

list(APPEND ENGINE_SRCS thirdparty/wrenbind/src/Runtime.cpp)
list(APPEND ENGINE_SRCS src/engine/display/CDisplay.cpp)
list(APPEND ENGINE_SRCS src/engine/application/Application.cpp)
list(APPEND ENGINE_SRCS thirdparty/wrenbind/src/Class.cpp)
list(APPEND ENGINE_SRCS src/engine/wren/wrenbind/WrenForwarders.cpp)
list(APPEND ENGINE_SRCS src/engine/wren/WrenManager.cpp)
list(APPEND ENGINE_SRCS ${WREN_BIND_GEN})

set_source_files_properties(
	${WREN_BIND_GEN}
	PROPERTIES GENERATED TRUE
)

if(APP STREQUAL "Demo")
	list(APPEND ENGINE_SRCS src/app/examples/example_demo/Demo.cpp)
else()
	list(APPEND ENGINE_SRCS src/app/examples/example_template/Template.cpp)
endif()

if(BUILD_PS3)
	add_definitions(-DPLATFORM_PS3 -mcpu=cell)

	set(CMAKE_SYSTEM_NAME Generic)
	set(CMAKE_C_COMPILER "$ENV{PS3DEV}/bin/ppu-gcc")
	set(CMAKE_CXX_COMPILER "$ENV{PS3DEV}/bin/ppu-g++")
	set(CMAKE_FIND_ROOT_PATH "$ENV{PS3DEV}/ppu" "$ENV{PSL1GHT}")
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -mcpu=cell -maltivec")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mcpu=cell -maltivec")

	include_directories(
		$ENV{PSL1GHT}/include
		$ENV{PS3DEV}/ppu/include
		/rsxgl/include
		${CMAKE_SOURCE_DIR}
		${CMAKE_SOURCE_DIR}/public
		${CMAKE_SOURCE_DIR}/src
		${CMAKE_SOURCE_DIR}/src/engine
		${CMAKE_SOURCE_DIR}/thirdparty
		${CMAKE_SOURCE_DIR}/thirdparty/mathsfury
		${CMAKE_SOURCE_DIR}/thirdparty/gcmgl/src
		${CMAKE_SOURCE_DIR}/thirdparty/gcmgl/public
		${CMAKE_SOURCE_DIR}/thirdparty/wren/src/include
		${CMAKE_SOURCE_DIR}/thirdparty/wrenbind/include
		${CMAKE_SOURCE_DIR}/thirdparty/wrenbind/public
	)

	link_directories(
		$ENV{PSL1GHT}/lib
		$ENV{PS3DEV}/ppu/lib
		/rsxgl/lib
	)

	list(APPEND ENGINE_SRCS
		thirdparty/gcmgl/src/renderer/gcm/GCMRenderer.cpp
		thirdparty/gcmgl/src/renderer/gcm/GCMBatchRenderer.cpp
		thirdparty/gcmgl/src/renderer/gcm/spu/SPUBatchTransformManager.cpp
		thirdparty/gcmgl/src/window/ps3/WindowPS3.cpp
		src/engine/input/ps3/InputPS3.cpp
		src/engine/audio/ps3/AudioPS3.cpp
	)

	add_library(engine STATIC ${ENGINE_SRCS})

	target_compile_definitions(engine PRIVATE
		WREN_OPT_META=0
		WREN_OPT_RANDOM=0
	)

	target_include_directories(engine PUBLIC
		${CMAKE_SOURCE_DIR}/include
		${CMAKE_SOURCE_DIR}/include/thirdparty
		${CMAKE_SOURCE_DIR}/src
		/rsxgl/include
		${CMAKE_SOURCE_DIR}/thirdparty/wren/src/include
	)

	target_link_libraries(engine
		GL
		EGL
		gcm_sys
		rsx
		psl1ght
		lv2
		m
		sysutil
		io
		audio
	)

	add_executable(codefury src/app/Main.cpp src/app/Engine.cpp)
	target_link_libraries(codefury engine)

	set_target_properties(codefury PROPERTIES
		SUFFIX ".elf"
		LINK_FLAGS "-Wl,-Map=codefury.map"
	)

	add_custom_target(content ALL
		COMMAND python3 ${CMAKE_SOURCE_DIR}/tools/build_content.py
			--platform ps3
			--output ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
		WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
		COMMENT "Building content"
	)
	add_dependencies(codefury content)
else()
	find_package(OpenGL REQUIRED)
	find_package(PkgConfig REQUIRED)
	pkg_check_modules(GLFW3 glfw3)
	if(NOT GLFW3_FOUND)
		message(FATAL_ERROR "Build failed, glfw3 not found")
	endif()

	add_library(glad STATIC thirdparty/glad/generated/src/gl.c)
	set_source_files_properties(thirdparty/glad/generated/src/gl.c PROPERTIES LANGUAGE C)

	target_include_directories(glad PUBLIC
		${CMAKE_SOURCE_DIR}/thirdparty/glad/generated/include
		${CMAKE_SOURCE_DIR}/thirdparty/glad/include
	)

	list(APPEND ENGINE_SRCS
		thirdparty/gcmgl/src/renderer/gl/GLRenderer.cpp
		thirdparty/gcmgl/src/renderer/gl/GLBatchRenderer.cpp
		thirdparty/gcmgl/src/window/glfw/WindowGLFW.cpp
		src/engine/input/glfw/InputGLFW.cpp
		src/engine/audio/miniaudio/AudioMiniaudio.cpp
		thirdparty/miniaudio/miniaudio.c
	)

	add_library(engine STATIC ${ENGINE_SRCS})

	target_compile_definitions(engine PRIVATE
		WREN_OPT_META=0
		WREN_OPT_RANDOM=0
		MA_NO_DECODING
		MA_NO_ENCODING
		MA_NO_GENERATION
		MA_NO_RESOURCE_MANAGER
		MA_NO_NODE_GRAPH
		MA_NO_ENGINE
	)

	target_include_directories(engine PUBLIC
		${CMAKE_SOURCE_DIR}
		${CMAKE_SOURCE_DIR}/public
		${CMAKE_SOURCE_DIR}/src
		${CMAKE_SOURCE_DIR}/src/engine
		${CMAKE_SOURCE_DIR}/thirdparty
		${CMAKE_SOURCE_DIR}/thirdparty/mathsfury
		${CMAKE_SOURCE_DIR}/thirdparty/gcmgl/src
		${CMAKE_SOURCE_DIR}/thirdparty/gcmgl/public
		${CMAKE_SOURCE_DIR}/thirdparty/simde
		${CMAKE_SOURCE_DIR}/thirdparty/wren/src/include
		${CMAKE_SOURCE_DIR}/thirdparty/wrenbind/include
		${CMAKE_SOURCE_DIR}/thirdparty/wrenbind/public
		${OPENGL_INCLUDE_DIR}
		${GLFW3_INCLUDE_DIRS}
	)

	target_link_libraries(engine PUBLIC
		OpenGL::GL
		${GLFW3_LIBRARIES}
		glad
		pthread
		m
		${CMAKE_DL_LIBS}
	)

	target_link_directories(engine PUBLIC ${GLFW3_LIBRARY_DIRS})

	add_executable(codefury src/app/Main.cpp src/app/Engine.cpp)
	target_link_libraries(codefury engine)

	add_custom_target(content ALL
		COMMAND python3 ${CMAKE_SOURCE_DIR}/tools/build_content.py
			--platform linux
			--output ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
		WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
		COMMENT "Building content"
	)
	add_dependencies(codefury content)
endif()