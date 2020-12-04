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

