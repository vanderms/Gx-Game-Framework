#include "GxNamespace.h"
#include "../Utilities/Util.h"
#include "../Array/Array.h"
#include "../Map/GxMap.h"
#include "../List/GxList.h"
#include "../Element/GxElement.h"
#include "../Renderable/GxRenderable.h"
#include "../RigidBody/GxRigidBody.h"
#include "../App/App.h"
#include "../Scene/GxScene.h"
#include "../Physics/GxPhysics.h"
#include "../Folder/GxFolder.h"
#include "../Graphics/GxGraphics.h"




const GxElemNamespace GxElemNamespaceInstance = {
	
	.create = GxCreateElement,	
	.createTilemap = NULL,	
	.remove = GxElemRemove,
	.getTarget = GxElemGetTarget,	
	.addRequestHandler = GxElemAddRequestHandler,
	.getID = GxElemGetId,
	.getScene = GxElemGetScene,
	.getPos = GxElemGetPosition,
	.setPos = GxElemSetPosition,
	.getCenter = GxElemGetCenter,	
	.getClassName = GxElemGetClassName,
	.hasClass = GxElemHasClass, 	
	.hasHandler = GxElemHasHandler,	
	.isPhysical = GxElemIsPhysical,
	.isRenderable = GxElemIsRenderable,
	.hasDynamicBody = GxElemHasDynamicBody,	
	.hasFixedBody = GxElemHasFixedBody,
	.hasRelativePosition = GxElemHasRelativePosition,
	.hasAbsolutePosition = GxElemHasAbsolutePosition,
	.setChild = GxElemSetChild,
	.getChild = GxElemGetChild,	
	//...body
	.isOnGround = GxElemIsOnGround,
	.getCmask = GxElemGetCmask,
	.setCmask = GxElemSetCmask,
	.getPreference = GxElemGetPreference,
	.setPreference = GxElemSetPreference,
	.hasFriction = GxElemHasFriction,
	.setFriction = GxElemSetFriction,	
	.getVelocity = GxElemGetVelocity,
	.setVelocity = GxElemSetVelocity,
	.getVely = GxElemGetVely,
	.setVely = GxElemSetVely,	
	.getVelx = GxElemGetVelx,
	.setVelx = GxElemSetVelx,
	.accelerate = GxElemAccelerate,
	.isMoving = GxElemIsMoving,
	.getElasticity = GxElemGetElasticity,
	.setElasticity = GxElemSetElasticity,
	.getRestitution = GxElemGetRestitution,
	.setRestitution = GxElemSetRestitution,
	.getMaxgvel = GxElemGetMaxgvel,
	.setMaxgvel = GxElemSetMaxgvel,
	.getContacts = GxElemGetContacts,
	.move = GxElemMove,
	.moveTo = GxElemMoveTo,
	//renderable	
	.getZIndex = GxElemGetZIndex,
	.setZIndex = GxElemSetZIndex,
	.getOpacity = GxElemGetOpacity,
	.setOpacity = GxElemSetOpacity,
	.getOrientation = GxElemGetOrientation,
	.setOrientation = GxElemSetOrientation,
	.getImage = GxElemGetImage,
	.setImage = GxElemSetImage,
	.getAnimation = GxElemGetAnimation,
	.setAnimation = GxElemSetAnimation,
	.getAlignment = GxElemGetAlignment,
	.setAlignment = GxElemSetAlignment,
	.isHidden = GxElemIsHidden,
	.hide = GxElemHide,
	.show = GxElemShow,
	.getAngle = GxElemGetAngle,
	.setAngle = GxElemSetAngle,
	.getProportion = GxElemGetProportion,
	.setProportion = GxElemSetProportion,
	.setToFit = GxElemSetToFit,
	.getBackgroundColor = GxElemGetBackgroundColor,
	.setBackgroundColor = GxElemSetBackgroundColor,
	.getBorderSize = GxElemGetBorderSize,
	.getBorderColor = GxElemGetBorderColor,
	.setBorder = GxElemSetBorder,
	.setText = GxElemSetText,	
	.getText = GxElemGetText,
	.setFontSize = GxElemSetFontSize,
	.getFontSize = GxElemGetFontSize,
	.setFont = GxElemSetFont,
	.getFont = GxElemGetFont,
	.getColor = GxElemGetColor, 
	.setColor = GxElemSetColor,
	.getPositionOnWindow = GxGetElemPositionOnWindow,		

	.NONE = GxElemNone,
	.ABSOLUTE = GxElemAbsolute,
	.RELATIVE = GxElemRelative,	
	.FIXED = GxElemFixed,
	.DYNAMIC = GxElemDynamic,
	.FORWARD = GxElemForward,
	.BACKWARD = GxElemBackward,
};


const GxFolderNamespace GxFolderNamespaceInstance = {
	.create = GxCreateFolder,
	.loadImage = GxLoadImage,
	.createTilemap = GxFolderCreateTilemap,
	.removeAsset = GxFolderRemoveAsset,
	.getImageSize = GxFolderGetImageSize,
	.loadTileset = GxLoadTileset,
	.createTiles = GxCreateTiles,
	.loadAnimation = GxLoadAnimation,
	.loadChunk = GxLoadChunk,
	.loadMusic = GxLoadMusic,
	.getMusic = GxFolderGetMusic,
	.getChunk = GxFolderGetChunk,
	.getTexture = GxFolderGetTexture,
};

const GxListNamespace GxListNamespaceInstance = {
	.create = GxCreateList,
	.destroy = GxDestroyList,
	.size = GxListSize,
	.first = GxListFirst,
	.last = GxListLast,
	.at = GxListAt,
	.contains = GxListContains,
	.begin = GxListBegin,
	.next = GxListNext,
	.push = GxListPush,
	.insert = GxListInsert,
	.replace = GxListReplace,
	.remove = GxListRemove,
	.removeByIndex = GxListRemoveByIndex,
	.clean = GxListClean
};

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
	.addRequestHandler = GxSceneAddRequestHandler,	
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

