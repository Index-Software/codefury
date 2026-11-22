class CEntityRegistry {
  foreign static createEntity()
  foreign static createEntity(name)
  foreign static createEntity(name, position)
  foreign static createEntity(name, position, rotation)
  foreign static createEntity(name, position, rotation, scale)
  foreign static findEntities(componentTypeName)
  foreign static findEntityByName(name)
  foreign static setEntityName(id, name)
  foreign static getEntityName(id)
  foreign static destroyEntity(id)

  foreign static addComponent(id, comp)

  foreign static removeComponents(id)

  foreign static getComponent(id, typeName)
  foreign static getComponentByType(id, typeName)

  foreign static setParent(id, parentId)
  foreign static getParent(id)
  foreign static getChildren(id)
}