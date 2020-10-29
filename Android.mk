include $(CLEAR_VARS)

LOCAL_MODULE := main

SDL_PATH := ../SDL

LOCAL_C_INCLUDES := $(LOCAL_PATH)/$(SDL_PATH)/include

# Add your application source files here...
LOCAL_SRC_FILES := ./main.c\
./Gx/App/GxApp.c\
./Gx/Array/GxArray.c\
./Gx/Button/GxButton.c\
./Gx/Element/GxElement.c\
./Gx/Event/GxEvent.c\
./Gx/Folder/GxFolder.c\
./Gx/Graphics/GxGraphics.c\
./Gx/List/GxList.c\
./Gx/Map/GxMap.c\
./Gx/Namespace/GxNamespace.c\
./Gx/Physics/GxPhysics.c\
./Gx/Quadtree/GxQuadtree.c\
./Gx/Renderable/GxRenderable.c\
./Gx/RigidBody/GxRigidBody.c\
./Gx/Scene/GxScene.c\
./Gx/Tilemap/GxTilemap.c\
./Gx/Utilities/GxUtil.c\

LOCAL_SHARED_LIBRARIES := SDL2 SDL2_image SDL2_ttf SDL2_mixer

LOCAL_LDLIBS := -lGLESv1_CM -lGLESv2 -llog

include $(BUILD_SHARED_LIBRARY)