#include "GxNamespace.h"
#include "../Utilities/Util.h"
#include "../Array/Array.h"
#include "../Map/GxMap.h"
#include "../List/List.h"
#include "../Element/Element.h"
#include "../App/App.h"
#include "../Scene/Scene.h"
#include "../Physics/GxPhysics.h"
#include "../Folder/Folder.h"
#include "../Graphics/Graphics.h"



const GxMapNamespace GxMapNamespaceInstance = {
	.create = GmCreateMap,
	.destroy = GxDestroyMap,
	.size = GxMapSize,
	.capacity = GxMapCapacity,
	.get = GxMapGet,
	.at = GxMapAt,
	.set = GxMapSet,
	.rehash = GxMapRehash,
	.remove = GxMapRemove,
	.removeByIndex = GxMapRemoveByIndex,
	.clean = GxMapClean
};

const GxContactNamespace GxContactNamespaceInstance = {
	.getColliding = GxContactGetColliding,
	.getCollided = GxContactGetCollided,
	.isBetween = GxContactIsBetween,
	.hasElement = GxContactHasElement,
	.hasDirection = GxContactHasDirection,
	.getDirection = GxContactGetDirection,
	.getOppositeElement = GxContactGetOppositeElement,
	.allowCollision = GxContactAllowCollision,
	.isElemRightContact = GxContactIsElemRightContact,
	.isElemLeftContact = GxContactIsElemLeftContact,
	.isElemDownContact = GxContactIsElemDownContact,
	.isElemUpContact = GxContactIsElemUpContact,
	.oneWayPlatform = GxContactOneWayPlatform,
	.RIGHT = GxContactRight,
	.LEFT = GxContactLeft,
	.HORIZONTAL = GxContactHorizontal,
	.UP = GxContactUp,
	.DOWN = GxContactDown,
	.VERTICAL = GxContactVertical,
	.ALL = GxContactAll,
	.CMASKNONE = GxCmaskNone,
	.CMASKALL = GxCmaskAll,
	.CMASKCAMERA = GxCmaskCamera,
	.CMASKDYNAMIC = GxCmaskDynamic,
	.CMASKFIXED = GxCmaskFixed,
};


const GxSceneNamespace GxSceneNamespaceInstance = {
	.create = GxCreateScene,		
	.getPercLoaded = GxSceneGetPercLoaded,
	.getName = GxSceneGetName,
	.getSize = GxSceneGetSize,
	.hasStatus = GxSceneHasStatus,
	.getStatus = GxSceneGetStatus,
	.getElem = GxSceneGetElement,
	.getGravity = GxSceneGetGravity,
	.hasGravity = GxSceneHasGravity,
	.getCamera = GxSceneGetCamera,	
	.setGravity = GxSceneSetGravity,
	.setTimeout = GxSceneSetTimeout,
	.addEventListener = GxSceneAddEventListener,
	.removeEventListener = GxSceneRemoveEventListener,
	.status = &(const struct GxStatusNamespace){
		.NONE = GxStatusNone,
		.LOADING = GxStatusLoading,
		.LOADED = GxStatusLoaded,
		.RUNNING = GxStatusRunning,		
		.UNLOADING = GxStatusLoading,
	},	
};

