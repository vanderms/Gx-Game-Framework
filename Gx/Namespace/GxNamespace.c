#include "GxNamespace.h"
#include "../Utilities/Util.h"
#include "../Array/Array.h"
#include "../Map/Map.h"
#include "../List/List.h"
#include "../Element/Element.h"
#include "../App/App.h"
#include "../Scene/Scene.h"
#include "../Physics/Physics.h"
#include "../Folder/Folder.h"
#include "../Graphics/Graphics.h"




const GxSceneNamespace GxSceneNamespaceInstance = {
	.create = nScene->create,		
	.getPercLoaded = nScene->getPercLoaded,
	.getName = nScene->name,
	.getSize = nScene->size,
	.hasStatus = nScene->hasStatus,
	.getStatus = nScene->status,
	.getElem = nScene->getElem,
	.getGravity = nScene->gravity,
	.hasGravity = nScene->hasGravity,
	.getCamera = nScene->getCamera,	
	.setGravity = nScene->setGravity,
	.setTimeout = nScene->setTimeout,
	.addEventListener = nScene->addListener,
	.removeEventListener = nScene->removeListener,	
};

