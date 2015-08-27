/****************************************************************************
 Copyright (c) 2015 Chukong Technologies Inc.

 http://www.cocos2d-x.org

 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.
 ****************************************************************************/

#pragma once

#include "cocos2d.h"

cocos2d::Scene* createSceneWithGame();

class GameNode : public cocos2d::Node
{
    enum ActorMode {
        RUNNING,
        JUMPING_UP,
        JUMPING_DOWN,
        GAMEOVER
    };

public:
    static GameNode* create();
    bool init();

protected:
    GameNode();
    virtual ~GameNode();

    virtual void update(float dt);
    void updateTilemap(float dt);
    void updateActor(float dt);
    void onTouchesEnded(const std::vector<cocos2d::Touch*>& touches, cocos2d::Event* event);
    void jump();

    cocos2d::TMXTiledMap* _tilemap1;
    cocos2d::TMXTiledMap* _tilemap2;

    cocos2d::Action* _runAction;
    cocos2d::Action* _jumpUpAction;
    cocos2d::Action* _jumpDownAction;

    float _gameSpeed;

    cocos2d::Sprite* _actor;
    ActorMode _actorMode;
    cocos2d::Vec2 _actorVel;
    float _accelTime;
};
