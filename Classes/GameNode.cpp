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

#include "GameNode.h"

using namespace cocos2d;

static const float FOREGROUND_SPEED = 250;      // pixels per second
static const float BACKGROUND_SPEED = 0.1;      // 10% of foreground speed
static const float ACTOR_POS_Y = 60;
static const float JUMP_VEL_Y = 4.5;
static const float GRAVITY_Y = 1.5;
static const float BUTTON_MAX_TIME = 0.5;       // max seconds that button can be pressed

enum {
    COIN = 0,
    BOX = 1,
    ANVIL = 2
};
Scene* createSceneWithGame()
{
    auto scene = Scene::create();
    auto game = GameNode::create();
    scene->addChild(game);
    return scene;
}

GameNode* GameNode::create()
{
    auto node = new (std::nothrow) GameNode();
    if (node && node->init())
    {
        node->autorelease();
    }
    return node;
}

GameNode::GameNode()
: _buttonMode(RELEASED)
, _elapsedTime(0)
, _buttonPressedTime(0)
, _gameSpeed(FOREGROUND_SPEED)
{
}

GameNode::~GameNode()
{
    CC_SAFE_RELEASE(_runAction);
    CC_SAFE_RELEASE(_jumpDownAction);
    CC_SAFE_RELEASE(_jumpUpAction);
}

bool GameNode::init()
{
    // assign size to node
    setContentSize(Director::getInstance()->getVisibleSize());

    // set background images
    // 2 sprites that will generate a fake "endless scroll"
    _background0 = Sprite::create("res/background00.png");
    _background1 = Sprite::create("res/background01.png");
    _background0->setAnchorPoint(Vec2::ZERO);
    _background1->setAnchorPoint(Vec2::ZERO);
    addChild(_background0);
    addChild(_background1);
    // _background1 right after _background0
    _background1->setPosition(Vec2(_background0->getContentSize().width,0));

    // set ground images
    // 2 sprites that will generate a fake "endless scroll"
    _ground0 = Sprite::create("res/ground00.png");
    _ground1 = Sprite::create("res/ground01.png");
    _ground0->setAnchorPoint(Vec2::ZERO);
    _ground1->setAnchorPoint(Vec2::ZERO);
    addChild(_ground0);
    addChild(_ground1);
    // _ground1 right after _ground0
    _ground1->setPosition(Vec2(_ground0->getContentSize().width,0));



    // add player animations for frame cache
    auto frameCache = SpriteFrameCache::getInstance();
    frameCache->addSpriteFramesWithFile("res/parkour.plist");

    // create main sprite
    _actor = Sprite::createWithSpriteFrameName("runner0.png");
    _actor->setAnchorPoint(Vec2(0.5,0));
    _actor->setPosition(128,ACTOR_POS_Y);
    addChild(_actor);

    // create the animations
    const char* animRun[] = {
        "runner0.png",
        "runner1.png",
        "runner2.png",
        "runner3.png",
        "runner4.png",
        "runner5.png",
        "runner6.png",
        "runner7.png",
    };
    const int RUN_FRAMES = sizeof(animRun)/sizeof(animRun[0]);

    const char* animJumpUp[] = {
        "runnerJumpUp0.png",
        "runnerJumpUp1.png",
        "runnerJumpUp2.png",
        "runnerJumpUp3.png",
    };
    const int JUMPUP_FRAMES = sizeof(animJumpUp)/sizeof(animJumpUp[0]);

    const char* animJumpDown[] = {
        "runnerJumpDown0.png",
        "runnerJumpDown1.png",
    };
    const int JUMPDOWN_FRAMES = sizeof(animJumpDown)/sizeof(animJumpDown[0]);

    Vector<SpriteFrame*> runFrames(RUN_FRAMES);
    for (int i=0; i<RUN_FRAMES; i++)
    {
        auto frame = frameCache->getSpriteFrameByName(animRun[i]);
        runFrames.pushBack(frame);
    }
    auto runAnimation = Animation::createWithSpriteFrames(runFrames, 0.1);

    Vector<SpriteFrame*> jumpUpFrames(JUMPUP_FRAMES);
    for (int i=0; i<JUMPUP_FRAMES; i++)
    {
        auto frame = frameCache->getSpriteFrameByName(animJumpUp[i]);
        jumpUpFrames.pushBack(frame);
    }
    auto jumpUpAnimation = Animation::createWithSpriteFrames(jumpUpFrames, 0.02);

    Vector<SpriteFrame*> jumpDownFrames(JUMPDOWN_FRAMES);
    for (int i=0; i<JUMPDOWN_FRAMES; i++)
    {
        auto frame = frameCache->getSpriteFrameByName(animJumpUp[i]);
        jumpDownFrames.pushBack(frame);
    }
    auto jumpDownAnimation = Animation::createWithSpriteFrames(jumpDownFrames, 0.2);


    // in order to animate the animation, we need to create an "Animate" action
    auto animate = Animate::create(runAnimation);
    _runAction = RepeatForever::create(animate);
    _jumpUpAction = Animate::create(jumpUpAnimation);
    _jumpDownAction = Animate::create(jumpDownAnimation);

    _runAction->retain();
    _jumpUpAction->retain();
    _jumpDownAction->retain();

    _actor->runAction(_runAction);
    _actorMode = ActorMode::RUNNING;

    // initial game speed. How many pixels per second
    _gameSpeed = FOREGROUND_SPEED;

    // trigger main loop
    scheduleUpdate();

    // call me when there are touches
    auto eventDispatcher = Director::getInstance()->getEventDispatcher();
    auto listener = EventListenerTouchAllAtOnce::create();
    listener->onTouchesBegan = CC_CALLBACK_2(GameNode::onTouchesBegan, this);
    listener->onTouchesEnded = CC_CALLBACK_2(GameNode::onTouchesEnded, this);
    eventDispatcher->addEventListenerWithSceneGraphPriority(listener, this);

    return true;
}

void GameNode::update(float dt)
{
    updateScroll(dt);
    updateActor(dt);
    updateObjects(dt);

    // accelerate game
    _gameSpeed += dt * 3;
}

void GameNode::updateScroll(float dt)
{
    // foreground and background moves at different speed

    //
    // Foreground
    //
    float dx = dt * _gameSpeed;

    // update ground
    auto pos1 = _ground0->getPosition();
    auto pos2 = _ground1->getPosition();
    pos1.x -= dx;
    pos2.x -= dx;

    if (pos1.x < -_ground0->getContentSize().width)
        pos1.x += _ground1->getContentSize().width * 2;
    if (pos2.x < -_ground1->getContentSize().width)
        pos2.x += _ground0->getContentSize().width * 2;

    _ground0->setPosition(pos1);
    _ground1->setPosition(pos2);

    //
    // Background
    //
    dx *= BACKGROUND_SPEED;

    pos1 = _background0->getPosition();
    pos2 = _background1->getPosition();
    pos1.x -= dx;
    pos2.x -= dx;

    if (pos1.x < -_background0->getContentSize().width)
        pos1.x += _background1->getContentSize().width * 2;
    if (pos2.x < -_background1->getContentSize().width)
        pos2.x += _background0->getContentSize().width * 2;

    _background0->setPosition(pos1);
    _background1->setPosition(pos2);
}

void GameNode::updateActor(float dt)
{
    if (_buttonMode == ButtonMode::PRESSED) {
        _buttonPressedTime += dt;

        if (_buttonPressedTime > BUTTON_MAX_TIME) {
            _buttonMode = ButtonMode::RELEASED;
            CCLOG("released");
        }
    }

    if (_actorMode==ActorMode::JUMPING_UP)
    {
        _accelTime += dt;

        // reduce gravity while button is pressed
        if (_buttonMode == ButtonMode::PRESSED)
            _actorVel.y -= (GRAVITY_Y *0.01) * _accelTime;
        else
            _actorVel.y -= GRAVITY_Y * _accelTime;

        Vec2 pos = _actor->getPosition();
        pos.y += _actorVel.y;

        // started going down ?
        if (_actorVel.y <= 0) {
            _actor->stopAllActions();
            _actor->runAction(_jumpDownAction);
            _actorMode = ActorMode::JUMPING_DOWN;
            _accelTime = 0;
        }

        _actor->setPosition(pos);
    }
    else if (_actorMode==ActorMode::JUMPING_DOWN)
    {
        _accelTime += dt;
        _actorVel.y -= GRAVITY_Y * _accelTime;

        Vec2 pos = _actor->getPosition();
        pos.y += _actorVel.y;

        if (pos.y <= ACTOR_POS_Y) {
            pos.y = ACTOR_POS_Y;
            _actor->stopAllActions();
            _actorMode = ActorMode::CROUCH;
            _actor->setSpriteFrame("runnerCrouch0.png");
            _elapsedTime = 0;
        }
        _actor->setPosition(pos);
    }
    else if (_actorMode==ActorMode::CROUCH)
    {
        _elapsedTime += dt;
        if (_elapsedTime >0.1) {
            _actor->stopAllActions();
            _actor->runAction(_runAction);
            _actorMode = ActorMode::RUNNING;
        }
    }
}

void GameNode::updateObjects(float dt)
{
    // 1 - scroll objects
    float dx = dt * _gameSpeed;
    for (Sprite* sprite: _objects)
    {
        auto pos = sprite->getPosition();
        sprite->setPosition(pos.x-dx,pos.y);
    }

    // 2 - remove objects no longer visible
    Vector<Sprite*> toRemove;

    // objects are ordered in X
    // if an object is still in the screen, it is safe
    // to assume that the rest of the objects are still inside the
    // screen too.
    for (const auto& object: _objects)
    {
        auto& pos = object->getPosition();
        auto& size = object->getContentSize();
        if (pos.x+size.width+50 < 0)
            toRemove.pushBack(object);
        else
            break;
    }

    for (const auto& object: toRemove)
        _objects.eraseObject(object);

    addObjects(dt);
}

void GameNode::onTouchesBegan(const std::vector<Touch*>& touches, Event* event)
{
    if (_actorMode == ActorMode::RUNNING || _actorMode == ActorMode::CROUCH) {
        jump();
        _buttonMode = ButtonMode::PRESSED;
        _buttonPressedTime = 0;
        CCLOG("pressed");
    }
}

void GameNode::onTouchesEnded(const std::vector<Touch*>& touches, Event* event)
{
    _buttonMode = ButtonMode::RELEASED;
    CCLOG("released");
}


void GameNode::jump()
{
    _actorMode = ActorMode::JUMPING_UP;
    _actor->stopAllActions();
    _actor->runAction(_jumpUpAction);
    _actorVel = Vec2(0, JUMP_VEL_Y);          // pixels per seconds going up
    _accelTime = 0;
}

void GameNode::addObjects(float dt)
{
    // if there are no more objects
    // on the screen, then add more.
    if (_objects.size()==0) {
        // there 10 different pattern for objects.
        // choose one at random
//        int pattern = CCRANDOM_0_1() * 10;

        const char* objects[5] = {
            "...........C...C..C.............AAA.....",
            "...........C...C..C.....................",
            "...........CCCCC..C.....................",
            "...B.......C...C..C.....B...............",
            "..BBB......C...C..C....BBB.............."
        };

        const int TOTAL_X = 40;
        const int TOTAL_Y = 5;

        for (int x=0; x<TOTAL_X; x++)
        {
            for (int y=0; y<TOTAL_Y; y++)
            {
                int yy = TOTAL_Y-y-1;
                char c = objects[y][x];
                if (c=='C')
                    addCoin(x,yy);
                else if (c=='B')
                    addBox(x,yy);
                else if (c=='A')
                    addAnvil(x,yy);
            }
        }
    }
}

void GameNode::addCoin(int x, int y)
{
    auto sprite = createObject(x, y, "coin0.png");
    sprite->setUserData((void*)COIN);
}
void GameNode::addBox(int x, int y)
{
    auto sprite = createObject(x, y, "box.png");
    sprite->setUserData((void*)BOX);
}
void GameNode::addAnvil(int x, int y)
{
    auto sprite = createObject(x, y, "anvil.png");
    sprite->setUserData((void*)ANVIL);
}

Sprite* GameNode::createObject(int x, int y, const std::string& spriteName)
{
    int SPACE_X = 56;
    int SPACE_Y = 44;

    auto sprite = Sprite::createWithSpriteFrameName(spriteName);
    sprite->setAnchorPoint(Vec2::ZERO);
    addChild(sprite);
    auto screenSize = Director::getInstance()->getVisibleSize();
    sprite->setPosition(screenSize.width + x * SPACE_X, ACTOR_POS_Y + y * SPACE_Y);

    _objects.pushBack(sprite);
    return sprite;
}
