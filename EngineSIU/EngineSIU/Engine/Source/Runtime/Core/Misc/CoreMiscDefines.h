#pragma once

// TODO: Editor 빌드 모드가 생기면 설정에 직접 만드는걸로
#define SIU_BUILD_EDITOR

#ifdef SIU_BUILD_EDITOR
    #define WITH_EDITOR 1
#else
    #define WITH_EDITOR 0
#endif
