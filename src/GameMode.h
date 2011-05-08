/*
 * GameMode is a data structure to consolidate all the things that go
 * into a particular game mode and set it all at once. This allows us
 * to develop different game modes on different files, and as long as
 * those declare a GameMode, it becomes just a matter of calling the
 * activate method on the GameMode that you want to switch into.
 */

#ifndef GAMEMODE_H_
#define GAMEMODE_H_

struct GameMode {
  void (*renderFunc)(void);
  void (*normalKeysDownFunc)(unsigned char, int, int);
  void (*normalKeysUpFunc)(unsigned char, int, int);
  void (*specialKeysDownFunc)(int, int, int);
  void (*specialKeysUpFunc)(int, int, int);
  void (*activeMotionFunc)(int, int);
  void (*passiveMotionFunc)(int, int);
  void (*joystickFunc)(unsigned int, int, int, int);
  void (*initializer)(void);

  GameMode(void (*setRenderFunc)(void),
           void (*setNormalKeysDownFunc)(unsigned char, int, int),
           void (*setNormalKeysUpFunc)(unsigned char, int, int),
           void (*setSpecialKeysDownFunc)(int, int, int),
           void (*setSpecialKeysUpFunc)(int, int, int),
           void (*setActiveMotionFunc)(int, int),
           void (*setPassiveMotionFunc)(int, int),
           void (*setJoystickFunc)(unsigned int, int, int, int),
           void (*setInitializer)(void)
           ) {
    renderFunc = setRenderFunc;
    normalKeysDownFunc = setNormalKeysDownFunc;
    normalKeysUpFunc = setNormalKeysUpFunc;
    specialKeysDownFunc = setSpecialKeysDownFunc;
    specialKeysUpFunc = setSpecialKeysUpFunc;
    activeMotionFunc = setActiveMotionFunc;
    passiveMotionFunc = setPassiveMotionFunc;
    joystickFunc = setJoystickFunc;
    initializer = setInitializer;
  }

  void activate() {
    glutDisplayFunc(renderFunc);
    glutIdleFunc(renderFunc);
    glutKeyboardFunc(normalKeysDownFunc);
    glutKeyboardUpFunc(normalKeysUpFunc);
    glutMotionFunc(activeMotionFunc);
    glutPassiveMotionFunc(passiveMotionFunc);

    glutSpecialFunc(specialKeysDownFunc);
    glutSpecialUpFunc(specialKeysUpFunc);
    glutJoystickFunc(joystickFunc, 10);
    initializer();
  }
};

#endif