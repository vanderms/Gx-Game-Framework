#include "GxNamespace.h"
#include "../Utilities/GxUtil.h"
#include "../Array/GxArray.h"
#include "../Map/GxMap.h"
#include "../List/GxList.h"
#include "../Element/GxElement.h"
#include "../Renderable/GxRenderable.h"
#include "../RigidBody/GxRigidBody.h"
#include "../App/GxApp.h"
#include "../Scene/GxScene.h"
#include "../Physics/GxPhysics.h"
#include "../Button/GxButton.h"
#include "../Folder/GxFolder.h"
#include "../Tilemap/GxTilemap.h"
#include "../Graphics/GxGraphics.h"

const GxAppNamespace GxAppNamespaceInstance = {
	.create = GxCreateApp,
	.run = GxAppRun,
	.SDL = &(struct GxSDLNamespace){
		.getWindow = GxGetSDLWindow,
		.getRenderer = GxGetSDLRenderer
	},
	.event = &(struct GxEventNamespace) {
		.LOAD = GxEventOnLoad,
		.LOOP_BEGIN = GxEventOnLoopBegin,
		.UPDATE = GxEventOnUpdate,
		.PRE_GRAPHICAL = GxEventOnPreGraphical,
		.PRE_RENDER = GxEventOnPreRender,
		.LOOP_END = GxEventOnLoopEnd,
		.UNLOAD = GxEventOnUnload,
		.KEYBOARD = GxEventOnKeyboard,
		.MOUSE = GxEventMouse,
		.FINGER = GxEventFinger,
		.SDL_DEFAULT = GxEventSDLDefault,
		.PRE_CONTACT = GxEventPreContact,
		.CONTACT_BEGIN = GxEventContactBegin,
		.CONTACT_END = GxEventContactEnd,
		.TIMEOUT = GxEventTimeout,
		.DESTROY = GxEventOnDestroy,
		.ELEM_REMOVAL = GxEventOnElemRemoval,
	},
	.getScene = GxGetScene,
	.getWindowSize = GxGetWindowSize,
	.loadScene = GxLoadScene,
	.addFont = GxAddFont,
	.getRunningScene = GxGetRunningScene,
	.getMainScene = GxGetMainScene,
	.alert = GxAlert,
	.runtimeError = GxRuntimeError,
	.playMusic = GxPlayMusic,
	.playChunk = GxPlayChunk,
	.stopMusic = Mix_HaltMusic,
	.pauseMusic = Mix_PauseMusic,
	.resumeMusic = Mix_ResumeMusic,
	.isPlayingMusic = Mix_PlayingMusic,
	.convertColor = GxConvertColor,	
	.tokenize = GxTokenize,
	.i = GxDataI,
	.u = GxDataU, 
	.f = GxDataF,
	.b = GxDataB,
	.c = GxDataC,
	.sf = GxDataSF,
	.ptr = GxDataPtr,
	.list = GxDataList,
	.array = GxDataArray,
	.map = GxDataMap,
	.rect = GxDataRect,
	.vector = GxDataVector,
	.point = GxDataPoint,
	.size = GxDataSize,
	.matrix = GxDataMatrix,	
};

const GxArrayNamespace GxArrayNamespaceInstance = {
	.create = GxCreateArray,
	.destroy = GxDestroyArray,
	.size = GxArraySize,
	.capacity = GxArrayCapacity,
	.at = GxArrayAt,
	.push = GxArrayPush,
	.insert = GxArrayInsert,
	.remove = GxArrayRemove,
	.removeByValue = GxArrayRemoveByValue,
	.indexOf = GxArrayIndexOf,
	.reserve = GxArrayReserve,
	.clean = GxArrayClean,
	.sort = GxArraySort	
};

const GxButtonNamespace GxButtonNamespaceInstance = {
	.create = GxCreateButton,
	.getStatus = GxButtonGetStatus,
	.hasStatus = GxButtonHasStatus,
	.KEYBOARD = GxButtonKeyboard,
	.FINGER = GxButtonFinger,
	.MOUSE = GxButtonMouse,	
	.NONE = GxButtonNone,
	.ON = GxButtonOn,
	.HOVER = GxButtonHover,
	.CLICK = GxButtonClick,
	.DOWN = GxButtonDown,
	.UP = GxButtonUp,
};

const GxElemNamespace GxElemNamespaceInstance = {
	
	.create = GxCreateElement,	
	.createTilemap = GxCreateTileMap,	
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
	.send = GxElemSend,
	.delegate = GxElemDelegate,

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
	.send = GxSceneSend,	
	.addRequestHandler = GxSceneAddRequestHandler,
	.delegate = GxSceneDelegate,
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

const GxUtilNamespace GxUtilNamespaceInstance = {
	.createInt = GxUtilCreateInt,
	.createUint = GxUtilCreateUint,
	.createBool = GxUtilCreateBool,
	.createDouble = GxUtilCreateDouble,
	.createString = GmCreateString,
	.createStringF = GmCreateStringF,
	.cloneString = GxCloneString,
	.split = GmArraySplit,
	.trim = GxTrim,
	.abs = GxAbs,
	.random = GxRandom,
	.printMask = GxPrintMask,
	.calcDistance = GxCalcDistance,
	.assertNullPointer = GxUserAssertNullPointer,
	.assertArgument = GxUserAssertArgumnent,
	.assertState = GxUserAssertState,
	.assertAlloc = GxUserAssertAlloc,
	.assertOutOfRange = GxUserAssertOutOfRange,
	.onDestroyFreeTarget = GxOnDestroyFreeTarget,
	.onDestroyDoNothing = GxOnDestroyDoNothing,
};
