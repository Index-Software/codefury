import "engine/renderer/shader" for CShader, ShaderType_t
import "engine/renderer/material" for CMaterial
import "engine/renderer/texture" for CTexture
import "engine/renderer/skybox" for CSkybox
import "engine/ecs/entity_registry" for CEntityRegistry
import "engine/ecs/components/camera" for CCamera
import "engine/ecs/components/mesh_builder" for CMeshBuilder, FaceOrientation_t
import "engine/ecs/components/light" for CLight, LightType_t
import "engine/scene" for CScene
import "engine/renderer/renderer" for IRenderer
import "engine/input/free_camera" for CFreeCamera
import "engine/renderer/ui" for CUI, Anchor_t
import "engine/debug" for CDebug
import "engine/color" for CColor
import "engine/maths/vector3" for CVector3

class App {
  construct new() {
    _camera = 0
    _freeCamera = 0
    _skybox = 0
  }

  init() {
    var litShader = CShader.new()
    litShader.createFromSource("lighting/Lit")

    var gridShader = CShader.new()
    gridShader.createFromSource("Grid")

    var skyboxShader = CShader.new()
    skyboxShader.createFromSource("skybox/SkyboxCubemap")
    skyboxShader.setType(ShaderType_t.Skybox)

    var skyboxTexture = CTexture.new()
    skyboxTexture.createCubemapFromEquirect(
      "assets/textures/environment/day_sky.jpg", 512)
    skyboxShader.addTexture(skyboxTexture, 0)

    var skyboxMaterial = CMaterial.new()
    skyboxMaterial.setShader(skyboxShader)

    _skybox = CSkybox.new()
    _skybox.init
    _skybox.setMaterial(skyboxMaterial)
    CScene.setSkybox(_skybox)

    var floorMaterial = CMaterial.new()
    floorMaterial.setShader(gridShader)
    floorMaterial.setFloat("Smoothness", 0.3)
    floorMaterial.setFloat("Metallic", 0.0)

    var floorEntity = CEntityRegistry.createEntity(
      "Floor",
      CVector3.new(0.0, -1.0, 0.0))
    var floorMesh = CMeshBuilder.createPlane(
      floorMaterial,
      250.0,
      CColor.new(0.055, 0.055, 0.055, 1.0),
      FaceOrientation_t.Outwards,
      1)
    CEntityRegistry.addComponent(floorEntity, floorMesh)
    CScene.addEntity(floorEntity)

    var defaultMaterial = CMaterial.new()
    defaultMaterial.setShader(litShader)
    defaultMaterial.setFloat("Smoothness", 0.5)
    defaultMaterial.setFloat("Metallic", 0.0)

    var sphereEntity = CEntityRegistry.createEntity(
      "Sphere",
      CVector3.new(0.0, -0.2, 0.0))
    var sphereMesh = CMeshBuilder.createSphere(
      defaultMaterial,
      0.8,
      16,
      32,
      CColor.new(0.85, 0.42, 0.38, 1.0),
      FaceOrientation_t.Outwards)
    CEntityRegistry.addComponent(sphereEntity, sphereMesh)
    CScene.addEntity(sphereEntity)

    var cubeEntity = CEntityRegistry.createEntity(
      "Cube",
      CVector3.new(3.0, 0.0, 0.0))
    var cubeMesh = CMeshBuilder.createCube(
      defaultMaterial,
      1.0,
      CColor.new(0.38, 0.62, 0.68, 1.0),
      FaceOrientation_t.Outwards,
      1)
    CEntityRegistry.addComponent(cubeEntity, cubeMesh)
    CScene.addEntity(cubeEntity)

    var directionalLightEntity = CEntityRegistry.createEntity(
      "DirectionalLight")
    var directionalLight = CLight.new()
    directionalLight.setType(LightType_t.Directional)
    directionalLight.setDirection(CVector3.new(-1.0, -1.0, -0.6))
    CEntityRegistry.addComponent(directionalLightEntity, directionalLight)
    CScene.addEntity(directionalLightEntity)

    var pointLightEntity = CEntityRegistry.createEntity(
      "PointLight",
      CVector3.new(-2.5, 4.0, 3.0))
    var pointLight = CLight.new()
    pointLight.setColor(CColor.new(0.55, 0.72, 1.0, 1.0))
    pointLight.setShadowStrength(0.35)
    CEntityRegistry.addComponent(pointLightEntity, pointLight)
    CScene.addEntity(pointLightEntity)

    var cameraEntity = CEntityRegistry.createEntity(
      "Camera",
      CVector3.new(1.5, 3.0, 12.0),
      CVector3.new(-0.85, -90.0, 0.0))
    _camera = CCamera.new()
    _camera.setPerspective(45.0, IRenderer.getAspectRatio(), 0.1, 2000.0)
    CEntityRegistry.addComponent(cameraEntity, _camera)

    _freeCamera = CFreeCamera.new()
    _freeCamera.setPosition(CVector3.new(1.5, 3.0, 12.0))
    _freeCamera.setRotation(CVector3.new(-0.85, -90.0, 0.0))
  }

  update(dt) {
    _freeCamera.update(_camera, dt)
    _camera.setPosition(_freeCamera.getPosition)
    _camera.setRotation(_freeCamera.getRotation)
    _camera.setPerspective(45.0, IRenderer.getAspectRatio(), 0.1, 2000.0)

    CScene.render()
  }
}