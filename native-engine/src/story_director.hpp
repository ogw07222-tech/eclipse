#pragma once
// ECLIPSE tutorial / prologue director (Docs/TUTORIAL_PROLOGUE_FLOW.md, 9 stages).
// Pure C++17: no SDL. Owns the stage state machine and drives Combat through its
// story hooks. Dialogue-free by design: it only emits glyph/arrow/fade state
// (StoryView) that Presentation draws; no text is produced here.
#include "combat.hpp"
#include "control_layout.hpp"
#include <cstdio>
#include <string>
#include <vector>

namespace eclipse {

enum class StoryStage {
    Practice=1,   // 1 control practice, no monsters
    Threshold,    // 2 castle entrance atmosphere
    FirstFoes,    // 3 first enemies (A1 wave, A2 gate breakthrough)
    Interior,     // 4 interior fodder + mid-boss (A3, A4)
    Core,         // 5 throne approach + king intro (A5)
    KingPhase1,   // 6 armored Demon King, beatable
    KingPhase2,   // 7 true form, narratively unavoidable defeat
    Death,        // 8 death, class loss, time reversal
    Forest,       // 9 starting forest, weakened baseline
    Free          // after the first reclaimed power: main game loop
};

// Everything Presentation needs, as plain numbers. Never text.
struct StoryView {
    int highlight=-1;          // ControlLayout button index to pulse, -1 none
    int arrow=0;               // 1 right, -1 left, 2 up (stick demonstration)
    int pips=0,pipsTotal=0;    // practice progress dots
    bool skipVisible=false;
    bool chevrons=false;       // path lights drawn along the ground
    float chevronX0=0,chevronX1=0;
    float gateGlow=0;          // pulsing exit portal cue
    float gateX=0;
    float blackout=0,whiteout=0,desaturate=0;
    float shake=0,flash=0;     // king armor-break feedback
    bool rewind=false;         // time-reversal motif
    float rewindPhase=0;
    float unlockBurst=0;       // reclaimed-power cue in the forest
    float wakeFrac=0;          // >0: protagonist rises (death pose played in reverse)
    int loopPips=0;            // regression loops shown so far, drawn as tally pips
    float memoryGlint=0;       // 0..1: the one memory that survives the last regression
    float altarGlow=0;         // memory tutorial: strong pulse on the memory altar
    int menuRow=-1;            // memory tutorial: stat row to pulse inside the altar menu
    float hintX=-1;            // memory tutorial end: world x of the foe whose memory to take (<0 none)
    // Control practice goals (no text): what to reach, what is being demonstrated.
    float freeze=0;            // >0: time is stopped at a reaction moment; only the shown button resumes it
    float previewR=0;          // freeze preview: radius around the player that the move will reach (0 none)
    float goalX=-1;            // world x of a beacon to walk to (<0 none)
    int goalCount=0,goalDone=0;// light orbs to collect (0 none)
    bool targetsHint=false;    // mark the practice targets
};

struct SpawnDef {int kind;float x;float wake;};
// Extra tutorial health scale per kind, on top of Combat::enemyPower. The gatekeeper knight is the
// only roster entry whose weakened health (218 at A3) was ~2.7x a soldier's; it is eased so
// stage 4 stays "extremely easy" (about 8 basic hits instead of 13). Everything else is unchanged.
inline float tutorialPower(int kind){return kind==GatekeeperKnight?.65f:1.f;}
// Tutorial-weakened versions of endgame enemies; positions respect each area's ground.
inline std::vector<SpawnDef> storyWave(int area){
    switch(area){
    case 1:return {{AshInfantry,1760,.0f},{AshInfantry,1880,.35f},{Gargoyle,2000,.7f},
                   {AshInfantry,2080,1.0f},{AshInfantry,1960,1.3f},{Gargoyle,2120,1.6f}};
    case 2:return {{AshInfantry,760,.0f},{AshInfantry,900,.3f},{AshInfantry,1040,.6f},
                   {DemonHound,1300,.9f},{DemonHound,1420,1.2f},{DemonHound,1540,1.5f},{DemonHound,1660,1.8f}};
    case 3:return {{GatekeeperKnight,780,.0f},{AshInfantry,650,.3f},{CastlePriest,1000,.6f},
                   {AshInfantry,900,.9f},{AshInfantry,1060,1.2f},{CastlePriest,1300,1.5f},
                   {AshInfantry,1330,1.8f},{GatekeeperKnight,1560,2.1f},{CastlePriest,1700,2.4f},{AshInfantry,1800,2.7f}};
    case 4:return {{Executioner,1300,.0f},{AshInfantry,700,.5f},{AshInfantry,850,.8f},
                   {AshInfantry,1000,1.1f},{AshInfantry,1600,1.4f}};
    default:return {{Gargoyle,1200,.0f},{AshInfantry,700,.4f},{AshInfantry,900,.7f},
                    {AshInfantry,1100,1.0f},{AshInfantry,1500,1.3f},{CastlePriest,1700,1.6f}};
    }
}

class StoryDirector {
public:
    // Combat tutorial beats, in order. Flash Executioner first, then the swap to the Ruler of Space.
    enum Beat {Walk,Jump,Dodge,Strike,Command,Blink,Finish,Rush,Mark,Verdict,Swap,Bolt,Spikes,Repulse,Field,Singularity,BeatCount};
    // control: button to pulse; arrow: stick hint; freeze: the prompt stops time at a reaction moment.
    struct BeatDef {int control;int arrow;bool freeze;};
    static constexpr BeatDef beats[BeatCount]={
        {ControlLayout::Stick,1,false},{ControlLayout::Stick,2,false},{ControlLayout::Dash,0,true},
        {ControlLayout::Attack,0,false},{ControlLayout::Sweep,0,false},{ControlLayout::Buff,0,true},
        {ControlLayout::Attack,0,false},{ControlLayout::Skill,0,true},{ControlLayout::Sweep,0,false},
        {ControlLayout::Ultimate,0,true},{ControlLayout::Swap,0,true},
        {ControlLayout::Attack,0,false},{ControlLayout::Sweep,0,false},{ControlLayout::Buff,0,true},
        {ControlLayout::Skill,0,false},{ControlLayout::Ultimate,0,true}};
    static constexpr int stepCount=BeatCount;
    static constexpr float failsafeSeconds=38.f;     // phase 2: guaranteed scripted defeat
    static constexpr float floorFailsafe=6.f;        // king at health floor this long: scripted defeat
    static constexpr float wakeSeconds=2.8f;
    // Infinite regression: the same five-room rewind (A5..A1) plays again and again, faster each time,
    // then once more slowly. Seconds per room for each loop; the last loop is the one that keeps a memory.
    static constexpr int regressionLoops=5;
    static constexpr float loopRoomSeconds[regressionLoops]={.6f,.4f,.25f,.15f,.8f};
    static constexpr float rewindStart=4.4f;
    static float rewindSeconds(){float t=0;for(float r:loopRoomSeconds)t+=5*r;return t;}
    static constexpr float skipX=640,skipY=22,skipW=80,skipH=48;

    StoryStage stage=StoryStage::Free;
    StoryView view;
    float cameraLead=0;
    struct SceneChange {bool pending=false;int area=0;bool forest=false;} scene;
    bool reincarnated=false;
    bool memoryTutorialDone=false;   // the first memory has been eaten at the altar (saved)
    std::string savePath;

    // ---- persistence -------------------------------------------------------
    void load(){
        if(savePath.empty())return;
        FILE* f=std::fopen(savePath.c_str(),"r");if(!f)return;
        int version=0,done=0,memory=0;
        if(std::fscanf(f,"ECLIPSE_STORY %d reincarnated %d",&version,&done)==2&&(version==1||version==2)){
            reincarnated=done!=0;
            memoryTutorialDone=version==1?reincarnated:(std::fscanf(f," memory %d",&memory)==1&&memory!=0);
        }
        std::fclose(f);
    }
    void save() const {
        if(savePath.empty())return;
        FILE* f=std::fopen(savePath.c_str(),"w");if(!f)return;
        std::fprintf(f,"ECLIPSE_STORY 2 reincarnated %d memory %d\n",reincarnated?1:0,memoryTutorialDone?1:0);
        std::fclose(f);
    }

    // ---- lifecycle ---------------------------------------------------------
    // Called when a new game begins. After the first reincarnation the prologue is skipped.
    void start(Combat& g){
        reset();
        if(reincarnated){beginForest(g);return;}
        g.scriptedSpawns=true;g.endOnClear=false;g.suppressGameOver=true;g.playerHealthFloor=1;
        g.dropsEnabled=false;            // the prologue is the legendary peak: no memory to eat yet
        g.beginRun();                    // a new game is a new run: no run upgrades carried over
        g.enemyPower=.45f;g.enemyDamageScale=.5f;
        g.kingPower=.70f;g.kingDamageScale=.55f;g.kingUnbeatable=true;
        g.handicap={};
        g.player.equippedJobs={{2,1}};   // the prologue always opens as the Flash Executioner, then the Ruler of Space
        g.configureCastleArea(1);
        stage=StoryStage::Practice;area=1;beat=0;
        requestScene(1);
        initPractice(g);
    }
    bool active() const {return stage!=StoryStage::Free;}
    bool portalOpen() const {return portalUnlocked;}
    // True while the story owns the player's input (staging beats, wake-up, death).
    bool inputLocked() const {return lockLeft>0||stage==StoryStage::Death;}
    float wakeRemaining() const {return wakeLeft;}
    void requestSkip(Combat& g){
        if(stage==StoryStage::Practice){beat=BeatCount;finishPractice(g);}
    }
    static bool skipHit(float x,float y){return x>=skipX&&x<skipX+skipW&&y>=skipY&&y<skipY+skipH;}

    // ---- per-tick: before Combat::step -------------------------------------
    void preStep(float dt,Combat& g,Actions& a){
        if(a.restart){a.restart=false;restartStage(g);}
        if(stage==StoryStage::Practice)practiceInput(g,a);
        if(stage==StoryStage::Forest&&view.unlockBurst>0&&a.skill)view.highlight=-1;
        if(inputLocked())a=Actions{};
        previous=a;(void)dt;
    }
    // Main loop reports the player crossed to another castle area through a portal.
    void onAreaChanged(Combat& g){
        area=g.castleArea;portalUnlocked=false;view.gateGlow=0;
        stage=area<=2?StoryStage::FirstFoes:area<=4?StoryStage::Interior:StoryStage::Core;
        spawnWave(g);
    }

    // ---- per-tick: after Combat::step --------------------------------------
    void postStep(float dt,Combat& g){
        time+=dt;lockLeft=down(lockLeft,dt);
        view.shake=down(view.shake,dt*1.6f);view.flash=down(view.flash,dt*2.2f);
        view.unlockBurst=down(view.unlockBurst,dt);
        float target=cameraTarget;
        if(pauseLeft>0){pauseLeft=down(pauseLeft,dt);target=340;if(pauseLeft<=0)cameraTarget=0;}
        cameraLead+=(target-cameraLead)*(1-std::exp(-3.f*dt));
        switch(stage){
        case StoryStage::Practice:    updatePractice(dt,g);break;
        case StoryStage::Threshold:   updateThreshold(g);break;
        case StoryStage::FirstFoes:
        case StoryStage::Interior:
        case StoryStage::Core:        updateWaves(dt,g);break;
        case StoryStage::KingPhase1:
        case StoryStage::KingPhase2:  updateKing(dt,g);break;
        case StoryStage::Death:       updateDeath(dt,g);break;
        case StoryStage::Forest:      updateForest(dt,g);break;
        case StoryStage::Free:        break;
        }
    }
    bool takeScene(SceneChange& out){if(!scene.pending)return false;out=scene;scene.pending=false;return true;}

private:
    // practice
    int beat=0,swapBase=0,hitBase=0;int hurtBase=0;Actions previous;
    bool prompted=false,frozen=false,tried=false,blinkSeen=false,straggle=false;
    float beatTime=0,resumeTime=0,triedAge=0,holdLeft=0;
    // flow
    int area=1;bool portalUnlocked=false,waveActive=false,pauseDone=false;
    float time=0,lockLeft=0,pauseLeft=0,cameraTarget=0,wakeLeft=0;
    // king
    unsigned seenShift=0,seenPhase2=0;float phase2Time=0,floorTime=0;
    // death sequence
    float deathT=0;int rewindSeg=-1;bool echoSwapped=false;
    // forest
    int forestFoes=0;float forestTime=0;
    bool giftGiven=false;int levelBaseline=0;float closeTimer=0;
    int deathSegment=-1;

    static float down(float v,float dt){return std::max(0.f,v-dt);}
    void reset(){
        stage=StoryStage::Free;view=StoryView{};cameraLead=0;scene=SceneChange{};
        beat=0;prompted=frozen=tried=blinkSeen=false;beatTime=resumeTime=triedAge=holdLeft=0;area=1;portalUnlocked=waveActive=pauseDone=false;
        time=lockLeft=pauseLeft=cameraTarget=wakeLeft=0;
        seenShift=seenPhase2=0;phase2Time=floorTime=0;deathT=0;rewindSeg=-1;echoSwapped=false;
        forestFoes=0;forestTime=0;giftGiven=false;levelBaseline=0;closeTimer=0;deathSegment=-1;previous=Actions{};
    }
    void requestScene(int a,bool forest=false){scene.pending=true;scene.area=a;scene.forest=forest;}
    static bool rising(bool now,bool before){return now&&!before;}

    // ---- stage 1: combat tutorial -----------------------------------------
    // One continuous fight on the castle approach; nothing is reset between beats. Basic tools get a soft
    // prompt (the button pulses while the fight goes on). Reaction moments stop time: the threat freezes
    // mid-swing, a preview shows what the move will do (path, target, reach) and only that button
    // resumes time, so the move fires straight into the situation it answers. Foes are real AI with a
    // health floor (Enemy::dummy) until the beat lets them fall. No text anywhere.
    static constexpr int orbGoal=3;
    static void press(Actions& a,int control){
        switch(control){
        case ControlLayout::Dash:a.dash=true;break;
        case ControlLayout::Attack:a.attack=true;break;
        case ControlLayout::Skill:a.skill=true;break;
        case ControlLayout::Sweep:a.skillSweep=true;break;
        case ControlLayout::Buff:a.skillBuff=true;break;
        case ControlLayout::Ultimate:a.ultimate=true;break;
        case ControlLayout::Swap:a.swap=true;break;
        default:break;}
    }
    static bool pressed(const Actions& a,int control){
        switch(control){
        case ControlLayout::Stick:return a.jump;
        case ControlLayout::Dash:return a.dash;
        case ControlLayout::Attack:return a.attack;
        case ControlLayout::Skill:return a.skill;
        case ControlLayout::Sweep:return a.skillSweep;
        case ControlLayout::Buff:return a.skillBuff;
        case ControlLayout::Ultimate:return a.ultimate;
        case ControlLayout::Swap:return a.swap;
        default:return false;}
    }
    static float clampX(float x){return std::clamp(x,220.f,1040.f);}
    static void spawnFoe(Combat& g,int kind,float x,float wake){
        g.spawn(kind,clampX(x),wake,1.f);
        auto& e=g.enemies.back();e.dummy=true;e.fights=true;
    }
    static int alive(const Combat& g,int kind=-1){
        int n=0;for(const auto& e:g.enemies)if(!e.dead()&&(kind<0||e.kind==kind))++n;return n;
    }
    // Spawn only what is missing, so R (replay this beat) and skipped set-ups still have their foes.
    static void ensure(Combat& g,int kind,int count,float x,float spacing,float wake){
        for(int i=alive(g,kind);i<count;++i)spawnFoe(g,kind,x+spacing*i,wake+.25f*i);
    }
    static Enemy* first(Combat& g,int kind){for(auto& e:g.enemies)if(!e.dead()&&e.kind==kind)return &e;return nullptr;}
    static Enemy* nearest(Combat& g){
        Enemy* best=nullptr;float d=1e9f;
        for(auto& e:g.enemies)if(!e.dead()&&e.wake<=0&&std::abs(e.x-g.player.x)<d){d=std::abs(e.x-g.player.x);best=&e;}
        return best;
    }
    static bool allAwake(const Combat& g){for(const auto& e:g.enemies)if(!e.dead()&&e.wake>0)return false;return true;}
    static int within(const Combat& g,float r){int n=0;for(const auto& e:g.enemies)if(!e.dead()&&std::abs(e.x-g.player.x)<r)++n;return n;}
    static bool allWithin(const Combat& g,float r){return within(g,r)==alive(g);}
    static void markAll(Combat& g){for(auto& e:g.enemies)if(!e.dead())e.lightMark=std::max(e.lightMark,6.f);}
    static void mortalAll(Combat& g,int hp){for(auto& e:g.enemies)if(!e.dead()){e.dummy=false;e.health=std::min(e.health,hp);}}
    static int hits(const Combat& g){int n=0;for(const auto& e:g.enemies)n+=e.dummyHits;return n;}
    bool anyMarked(const Combat& g) const {for(const auto& e:g.enemies)if(!e.dead()&&e.lightMark>0)return true;return false;}
    static void clearCooldowns(Player& p){
        p.skillCooldown=p.sweepReady=p.buffReady=p.verdictCooldown=p.attackCooldown=0;
        p.dashCooldown=p.mageSkillCooldown=p.swapCooldown=0;
    }

    void initPractice(Combat& g){
        g.enemies.clear();view.skipVisible=true;view.pipsTotal=BeatCount;beat=0;
        enterBeat(g);
    }
    // Sets the scene for the current beat. Foes carry over from beat to beat; only what is missing is added.
    void enterBeat(Combat& g){
        prompted=frozen=tried=blinkSeen=straggle=false;beatTime=resumeTime=triedAge=holdLeft=0;
        g.timeScale=1;
        view.freeze=0;view.previewR=0;view.hintX=-1;view.goalX=-1;view.goalCount=0;view.targetsHint=false;view.chevrons=false;
        view.highlight=-1;view.arrow=0;view.pips=beat;
        if(beat>=BeatCount)return;
        const float x=g.player.x;
        hitBase=hits(g);hurtBase=g.player.hurtSerial;swapBase=g.player.activeJob;
        switch(beat){
        case Walk:view.goalX=clampX(x+320);break;
        case Dodge:ensure(g,AshInfantry,1,x+260,0,.9f);break;               // a soldier rises and closes in
        case Strike:ensure(g,AshInfantry,1,x+160,0,.4f);view.goalCount=orbGoal;break;
        case Command:                                                         // reinforcements and a caster at range
            ensure(g,AshInfantry,2,x+300,90,.5f);ensure(g,CastlePriest,1,x+600,0,.9f);break;
        case Blink:case Finish:ensure(g,CastlePriest,1,x+600,0,.4f);break;
        case Rush:ensure(g,AshInfantry,2,x-320,-90,.4f);break;
        case Mark:                                                            // a wave from both sides
            spawnFoe(g,AshInfantry,x+260,.3f);spawnFoe(g,AshInfantry,x-260,.5f);spawnFoe(g,DemonHound,x+430,.7f);
            spawnFoe(g,DemonHound,x-430,.9f);spawnFoe(g,AshInfantry,x+520,1.1f);break;
        case Verdict:ensure(g,AshInfantry,1,x+300,0,.3f);break;
        case Swap:                                                            // something heavy arrives
            spawnFoe(g,GatekeeperKnight,x+360,1.2f);spawnFoe(g,AshInfantry,x+520,.6f);spawnFoe(g,AshInfantry,x-300,.9f);break;
        case Bolt:case Spikes:case Repulse:case Field:case Singularity:ensure(g,AshInfantry,2,x+300,90,.4f);break;
        default:break;}
    }
    // When the prompt appears. Soft beats prompt at once; reaction beats wait for their moment.
    bool trigger(Combat& g){
        const auto& p=g.player;
        switch(beat){
        case Dodge:
            for(const auto& e:g.enemies)if(!e.dead()&&e.fights&&e.attackAge>=.40f&&std::abs(e.x-p.x)<240)return true;
            return false;
        case Command:case Mark:case Swap:return allAwake(g)&&beatTime>.4f;
        case Blink:{const Enemy* c=first(g,CastlePriest);return !c||c->attackAge>=0||beatTime>4.f;}
        case Rush:return within(g,320)>0||beatTime>5.f;
        case Verdict:return beatTime>.3f;
        case Repulse:return within(g,190)>=2||beatTime>5.f;
        case Singularity:return allWithin(g,330)||beatTime>4.f;
        default:return true;
        }
    }
    void onPrompt(Combat& g){
        auto& p=g.player;
        if(beats[beat].freeze){
            frozen=true;g.timeScale=0;view.freeze=1;view.flash=std::max(view.flash,.25f);
            clearCooldowns(p);hurtBase=p.hurtSerial;
            if(p.flashMove==FlashMove::Slash){p.flashMove=FlashMove::None;p.flashAge=0;}   // a swing in progress never eats the answer
        }
        Enemy* n=nearest(g);
        auto faceNearest=[&]{if(n)p.facing=n->x>=p.x?1:-1;};
        switch(beat){
        case Dodge:{faceNearest();const float to=p.x+p.facing*230.f;          // dash straight through the blow
            view.chevrons=true;view.chevronX0=std::min(p.x,to);view.chevronX1=std::max(p.x,to);break;}
        case Strike:case Bolt:case Spikes:case Field:view.targetsHint=true;break;
        case Command:if(p.storedLightLances<orbGoal)p.storedLightLances=orbGoal;view.targetsHint=true;break;
        case Blink:if(Enemy* c=first(g,CastlePriest))view.hintX=c->x;break;   // the farthest marked foe
        case Finish:if(Enemy* c=first(g,CastlePriest)){c->dummy=false;view.hintX=c->x;}break;
        case Rush:{faceNearest();mortalAll(g,12);const float to=p.x+p.facing*340.f;
            view.chevrons=true;view.chevronX0=std::min(p.x,to);view.chevronX1=std::max(p.x,to);break;}
        case Mark:p.storedLightLances=std::max(p.storedLightLances,5);view.targetsHint=true;break;
        case Verdict:markAll(g);mortalAll(g,30);view.targetsHint=true;break;  // 20 lances, about 4 each
        case Swap:if(Enemy* k=first(g,GatekeeperKnight))view.hintX=k->x;break;
        case Repulse:view.previewR=180;break;
        case Singularity:view.previewR=340;mortalAll(g,40);break;
        default:break;
        }
    }
    // Called from preStep: while time is stopped only the prompted button gets through.
    // Only the beat's own button works (plus walking): every other action button is dead, so the player
    // can never answer a moment with the wrong move, spend the orbs early or swap away mid-lesson.
    void practiceInput(Combat& g,Actions& a){
        if(beat>=BeatCount){return;}
        const Actions raw=a;
        practiceCore(g,a);
        const int c=beats[beat].control;
        const bool open=holdLeft<=0;                       // between beats nothing but walking is live
        const Actions kept=a;
        a.dash=a.attack=a.skill=a.skillSweep=a.skillBuff=a.ultimate=a.swap=false;
        if(c!=ControlLayout::Stick)a.jump=a.jumpHeld=false;
        if(open){
            press2(a,kept,c);
            if(straggle)a.attack=kept.attack;              // the finale's stragglers are finished by hand
        }
        (void)raw;
    }
    static void press2(Actions& out,const Actions& from,int control){
        switch(control){
        case ControlLayout::Dash:out.dash=from.dash;break;
        case ControlLayout::Attack:out.attack=from.attack;break;
        case ControlLayout::Skill:out.skill=from.skill;break;
        case ControlLayout::Sweep:out.skillSweep=from.skillSweep;break;
        case ControlLayout::Buff:out.skillBuff=from.skillBuff;break;
        case ControlLayout::Ultimate:out.ultimate=from.ultimate;break;
        case ControlLayout::Swap:out.swap=from.swap;break;
        default:break;}
    }
    void practiceCore(Combat& g,Actions& a){
        if(beat>=BeatCount||holdLeft>0)return;
        const int c=beats[beat].control;
        const bool now=rising(pressed(a,c),pressed(previous,c));
        if(frozen){
            a=Actions{};
            if(now){press(a,c);frozen=false;g.timeScale=1;view.freeze=0;view.previewR=0;resumeTime=0;tried=true;triedAge=0;
                    if(beat==Dodge||beat==Rush)view.chevrons=false;}
            return;
        }
        if(prompted&&now&&!tried){tried=true;triedAge=0;}
    }
    bool success(Combat& g){
        const auto& p=g.player;
        switch(beat){
        case Walk:return std::abs(p.x-view.goalX)<50.f;
        case Jump:return tried&&!p.grounded;
        case Dodge:{
            if(!tried||resumeTime<.1f)return false;
            for(const auto& e:g.enemies)if(!e.dead()&&e.attackAge>=0)return false;   // wait for the blow to finish
            return p.hurtSerial==hurtBase;
        }
        case Strike:return p.storedLightLances>=orbGoal;
        case Command:case Mark:return anyMarked(g);
        case Blink:return tried&&blinkSeen&&p.flashMove==FlashMove::None;
        case Finish:return alive(g,CastlePriest)==0;
        case Rush:case Verdict:case Singularity:return alive(g)==0;
        case Swap:return p.activeJob!=swapBase;
        case Bolt:return hits(g)>hitBase;
        case Spikes:return tried&&(triedAge>2.f||(triedAge>.6f&&hits(g)>hitBase));
        case Repulse:return tried&&triedAge>.6f;
        case Field:return tried&&triedAge>.8f;
        default:return false;
        }
    }
    void onSuccess(Combat& g){
        if(beat==Command||beat==Mark)markAll(g);                 // the lances' light spreads to every foe in sight
        if(beat==Finish)g.player.storedLightLances=0;
    }
    static float holdFor(int b){
        switch(b){case Verdict:return 2.4f;case Singularity:return 2.2f;case Rush:return 1.f;case Dodge:return .8f;default:return .5f;}
    }
    void updatePractice(float dt,Combat& g){
        auto& p=g.player;p.health=p.maxHealth();
        if(beat>=BeatCount){finishPractice(g);return;}
        if(frozen){view.highlight=beats[beat].control;view.arrow=beats[beat].arrow;return;}   // time stands still
        beatTime+=dt;if(prompted)resumeTime+=dt;if(tried)triedAge+=dt;
        if(p.flashMove==FlashMove::Blink)blinkSeen=true;
        if(beat==Blink||beat==Finish||beat==Rush||beat==Verdict)markAll(g);   // marked foes stay marked for Blink and Verdict
        if(holdLeft>0){
            holdLeft=std::max(0.f,holdLeft-dt);
            if(holdLeft<=0){++beat;enterBeat(g);}
            return;
        }
        if(beat==Finish)if(Enemy* c=first(g,CastlePriest))view.hintX=c->x;
        view.goalDone=p.storedLightLances;
        if(!prompted&&trigger(g)){prompted=true;onPrompt(g);if(frozen){view.highlight=beats[beat].control;return;}}
        view.highlight=!prompted?-1:straggle?ControlLayout::Attack:beats[beat].control;view.arrow=prompted?beats[beat].arrow:0;
        if(!prompted)return;
        if(success(g)){
            onSuccess(g);holdLeft=holdFor(beat);view.pips=beat+1;
            view.highlight=-1;view.arrow=0;view.goalX=-1;view.hintX=-1;view.targetsHint=false;view.chevrons=false;view.previewR=0;
            return;
        }
        // A reaction that did not answer the moment (hit anyway, missed, nothing left in range): the moment comes again.
        if(beats[beat].freeze&&tried){
            const bool hurt=beat==Dodge&&p.hurtSerial!=hurtBase;
            const float retry=beat==Verdict?4.f:beat==Singularity?3.f:1.4f;
            if(hurt||resumeTime>retry){
                if(beat==Singularity){mortalAll(g,1);straggle=true;}   // finish the stragglers by hand
                else {prompted=tried=false;beatTime=std::max(beatTime,5.f);}
            }
        }
    }
    void finishPractice(Combat& g){
        view.skipVisible=false;view.highlight=-1;view.arrow=0;view.pips=view.pipsTotal=0;
        view.freeze=0;view.previewR=0;view.hintX=-1;view.goalX=-1;view.goalCount=view.goalDone=0;view.targetsHint=false;view.chevrons=false;
        frozen=prompted=false;holdLeft=0;g.timeScale=1;
        stage=StoryStage::Threshold;pauseDone=false;cameraTarget=0;
        g.enemies.clear();g.lightLances.clear();g.projectiles.clear();
        // The prologue always continues as the Flash Executioner, whichever class the tutorial ended on.
        auto& p=g.player;
        p.flashMove=FlashMove::None;p.flashAge=0;p.spaceMove=0;p.spaceField=0;p.dashLeft=0;p.meleeLeft=0;
        const bool swapped=p.activeJob!=0;
        p.activeJob=0;const int job=p.equippedJobs[0];p.flashExecutioner=job==2;p.form=job==1?1:0;
        clearCooldowns(p);g.refreshStats();p.storedLightLances=0;
        if(swapped&&p.flashExecutioner)g.lightBursts.push_back({p.x,p.y-48,.24f,LightFx::Arrive,p.facing,0});
        // The threshold beat (pause at x=1100, combat trigger at x=1480) must always be played: a player
        // already past it is brought back to the start, as restartStage() does.
        if(p.x>=1050.f)p.x=CastleTerrain::profile(1).spawnX;
        p.vx=0;p.facing=1;
    }

    // ---- stage 2: castle threshold -----------------------------------------
    void updateThreshold(Combat& g){
        const float px=g.player.x;
        view.chevrons=px<1460;view.chevronX0=px+90;view.chevronX1=1500;
        if(!pauseDone&&px>=1100){pauseDone=true;pauseLeft=1.8f;lockLeft=1.8f;g.player.facing=1;}
        if(px>=1480){view.chevrons=false;stage=StoryStage::FirstFoes;spawnWave(g);}
    }

    // ---- stages 3-5: fodder waves ------------------------------------------
    void spawnWave(Combat& g){
        g.enemies.clear();g.scriptedSpawns=true;
        for(const auto& s:storyWave(area)){
            g.spawn(s.kind,s.x,s.wake,tutorialPower(s.kind));
            if(area==4&&s.kind==Executioner)g.enemies.back().boss=true; // mid-boss reads as a boss
        }
        waveActive=true;portalUnlocked=false;view.gateGlow=0;view.chevrons=false;
    }
    void updateWaves(float dt,Combat& g){
        (void)dt;
        if(waveActive&&g.foes()==0){
            waveActive=false;
            if(area<5){portalUnlocked=true;view.gateX=CastleTerrain::portalX(area,true);}
            else beginKing(g);
        }
        if(portalUnlocked){
            view.gateGlow=1;view.chevrons=g.player.x<view.gateX-120;
            view.chevronX0=g.player.x+90;view.chevronX1=view.gateX;
        }
    }
    void beginKing(Combat& g){
        stage=StoryStage::KingPhase1;
        g.spawn(DemonKingArmored,1800,3.2f);
        lockLeft=2.4f;pauseLeft=3.4f;g.player.facing=1;
        seenShift=g.kingShiftSerial;seenPhase2=g.kingPhase2Serial;phase2Time=floorTime=0;
    }

    // ---- stages 6-7: Demon King --------------------------------------------
    const Enemy* king(const Combat& g) const {
        for(const auto& e:g.enemies)if(e.kind==DemonKingArmored||e.kind==DemonKingTrue)return &e;
        return nullptr;
    }
    void updateKing(float dt,Combat& g){
        if(g.kingShiftSerial!=seenShift){seenShift=g.kingShiftSerial;view.shake=1.f;view.flash=.9f;}
        if(g.kingPhase2Serial!=seenPhase2){
            seenPhase2=g.kingPhase2Serial;stage=StoryStage::KingPhase2;
            g.playerHealthFloor=0;           // from here the player can truly fall
            view.flash=1.f;view.shake=.8f;phase2Time=floorTime=0;
        }
        if(stage!=StoryStage::KingPhase2)return;
        phase2Time+=dt;
        const Enemy* k=king(g);
        if(k&&k->health<=k->maxHealth*35/100+1)floorTime+=dt;else floorTime=0;
        const bool finale=g.defeated||phase2Time>=failsafeSeconds||floorTime>=floorFailsafe;
        if(finale){
            if(!g.defeated)g.defeatPlayer();
            beginDeath(g);
        }
    }

    // ---- stage 8: death, class loss, infinite regression ---------------------
    // Timeline (seconds): 0-1.6 death pose in silence, 1.6-3.4 class echoes flicker out, 3.4-4.4 blackout,
    // then the regression loops (rewindSeconds() long: four fast loops, one slow loop that keeps a memory),
    // then a whiteout and the forest.
    void beginDeath(Combat& g){
        (void)g;stage=StoryStage::Death;deathT=0;rewindSeg=-1;deathSegment=-1;echoSwapped=false;
        view.flash=1.f;view.shake=1.f;
    }
    // Which loop and room a time inside the rewind falls in; phase 0..1 inside that room.
    static void locateRewind(float local,int& loop,int& room,float& phase,float& loopStart){
        float acc=0;
        for(int l=0;l<regressionLoops;++l){
            const float seg=loopRoomSeconds[l];
            for(int r=0;r<5;++r){
                if(local<acc+seg){loop=l;room=r;phase=(local-acc)/seg;return;}
                acc+=seg;
            }
            loopStart=acc;
        }
        loop=regressionLoops-1;room=4;phase=1.f;
    }
    void updateDeath(float dt,Combat& g){
        deathT+=dt;
        const float t=deathT,rewindEnd=rewindStart+rewindSeconds();
        if(t>=1.6f&&t<3.4f){
            view.desaturate=std::min(.85f,(t-1.6f)/1.8f);
            // The two legendary classes flicker and extinguish.
            const int tick=static_cast<int>((t-1.6f)*5.5f);
            const int job=g.player.equippedJobs[(tick&1)?1:0];
            g.player.flashExecutioner=job==2;g.player.form=job==1?1:0;
        }
        view.blackout=t<3.4f?0.f:t<rewindStart?(t-3.4f):.35f;
        if(t>=rewindStart&&t<rewindEnd){
            view.rewind=true;
            int loop=0,room=0;float phase=0,loopStart=0;
            locateRewind(t-rewindStart,loop,room,phase,loopStart);
            view.rewindPhase=phase;
            view.loopPips=loop+1;                       // one more tally mark every loop
            if(loop==regressionLoops-1){
                // The last regression: the world dims and thins out, one memory stays lit.
                float finalStart=0;for(int l=0;l<regressionLoops-1;++l)finalStart+=5*loopRoomSeconds[l];
                const float finalLen=5*loopRoomSeconds[regressionLoops-1];
                view.memoryGlint=std::clamp((t-rewindStart-finalStart)/finalLen,0.f,1.f);
                view.blackout=.35f+.25f*view.memoryGlint;
            }
            const int segment=loop*5+room;
            if(segment!=deathSegment){
                if(room==0&&loop>0){view.flash=.9f;view.shake=.6f;}   // the loop snaps back to the start
                deathSegment=segment;
                static constexpr int order[5]={5,4,3,2,1};
                const int next=order[room];
                g.transitionCastleArea(next,next==5?900.f:760.f);
                g.player.health=0;
                requestScene(next);
            }
        }
        if(t>=rewindEnd){view.rewind=false;view.whiteout=std::min(1.f,(t-rewindEnd)/.8f);}
        if(t>=rewindEnd+.8f)beginForest(g);
    }

    // ---- stage 9: starting forest ------------------------------------------
    void beginForest(Combat& g){
        stage=StoryStage::Forest;
        g.scriptedSpawns=false;g.endOnClear=false;g.suppressGameOver=false;g.playerHealthFloor=0;
        g.dropsEnabled=true;             // from the starting forest on, fallen enemies leave memories
        g.beginRun();                    // a regression starts a fresh run: run upgrades are gone
        g.enemyPower=1.f;g.enemyDamageScale=1.f;g.defeated=false;
        g.handicap={true,true,0,.5f,.72f};
        g.player.equippedJobs={{3,3}};        // blade baseline only; legendary classes are lost
        g.enterForest();
        g.player.health=45;g.player.trailingHealth=45;
        view=StoryView{};view.whiteout=1.f;
        view.chevrons=true;
        reincarnated=true;save();
        wakeLeft=wakeSeconds;lockLeft=wakeSeconds;cameraLead=0;cameraTarget=0;pauseLeft=0;
        forestFoes=g.foes();forestTime=0;
        giftGiven=false;levelBaseline=totalLevels(g.progression);closeTimer=0;
        requestScene(0,true);
    }
    static int totalLevels(const Progression& p){
        int total=0;for(const auto& job:p.levels)for(int level:job)total+=level;return total;
    }
    // Memory tutorial (first reincarnation only): a memory of the earlier regressions remains. The altar glows,
    // the interact prompt pulses, the menu points at the attack row; once it is eaten the way to the next foe's
    // memory is shown. Light and glyphs only: nothing to read.
    void updateMemoryTutorial(float dt,Combat& g){
        view.altarGlow=0;view.menuRow=-1;view.hintX=-1;
        if(wakeLeft>0)return;
        if(!giftGiven){
            giftGiven=true;
            const int need=Progression::upgradeCost(0)-g.progression.memories;
            if(need>0)g.progression.addMemories(need);   // one memory survived the regressions
        }
        if(totalLevels(g.progression)<=levelBaseline){
            if(g.memoryMenu){view.menuRow=0;view.highlight=-1;}
            else{view.highlight=ControlLayout::Attack;view.altarGlow=1.f;}
            return;
        }
        // Eaten: close the menu by itself, then point at the nearest living foe.
        if(g.memoryMenu){closeTimer+=dt;if(closeTimer>=.9f)g.memoryMenu=false;return;}
        closeTimer=0;view.highlight=-1;
        float bestX=-1,bestDistance=1e9f;
        for(const auto& e:g.enemies){
            if(e.dead())continue;
            const float distance=std::abs(e.x-g.player.x);
            if(distance<bestDistance){bestDistance=distance;bestX=e.x;}
        }
        view.hintX=bestX;
    }
    void updateForest(float dt,Combat& g){
        forestTime+=dt;
        wakeLeft=down(wakeLeft,dt);
        view.wakeFrac=wakeSeconds>0?wakeLeft/wakeSeconds:0;
        view.whiteout=down(view.whiteout,dt*.9f);
        // Accessible cue: light motes lead to the first foe; defeating it returns the first power.
        view.chevronX0=g.player.x+90;view.chevronX1=std::min(1500.f,g.player.x+620.f);
        if(!memoryTutorialDone)updateMemoryTutorial(dt,g);
        const bool ateFirst=memoryTutorialDone||totalLevels(g.progression)>levelBaseline;
        view.chevrons=g.handicap.skills==0&&wakeLeft<=0&&ateFirst&&!g.memoryMenu;
        if(g.handicap.skills==0&&g.foes()<forestFoes){
            g.handicap.skills=1;g.applyHandicap();
            view.unlockBurst=2.4f;view.highlight=ControlLayout::Skill;view.chevrons=false;
        }
        if(g.handicap.skills>=1&&view.unlockBurst<=0&&ateFirst){
            view.highlight=-1;view.hintX=-1;view.altarGlow=0;view.menuRow=-1;
            memoryTutorialDone=true;save();   // the tutorial ends: memories are what make the protagonist stronger
            stage=StoryStage::Free;
        }
    }

    // ---- restart (R key): replay the current beat, never the whole story ----
    void restartStage(Combat& g){
        if(stage==StoryStage::Death)return;   // the scripted loss is never skipped: keep it frozen and lethal
        g.defeated=false;g.ended=false;g.won=false;
        g.player.health=g.player.maxHealth();g.player.trailingHealth=float(g.player.health);
        switch(stage){
        case StoryStage::Practice:
            g.timeScale=1;frozen=false;enterBeat(g);break;   // replay the current beat
        case StoryStage::Threshold:
            g.player.x=CastleTerrain::profile(1).spawnX;pauseDone=false;break;
        case StoryStage::FirstFoes:case StoryStage::Interior:case StoryStage::Core:
            g.player.x=CastleTerrain::arrivalX(area);spawnWave(g);break;
        case StoryStage::KingPhase1:case StoryStage::KingPhase2:
            stage=StoryStage::KingPhase1;g.playerHealthFloor=1;g.enemies.clear();
            g.player.x=CastleTerrain::arrivalX(5);beginKing(g);break;
        case StoryStage::Death:break;  // the scripted loss is never skipped by restart
        case StoryStage::Forest:
            // Same weakened baseline as beginForest(): weaker foes and the 45 HP start.
            g.restart();g.weakenForestFoes();g.player.health=45;g.player.trailingHealth=45;break;
        case StoryStage::Free:
            g.restart();g.player.health=std::max(45,g.player.health);break;
        }
    }
};

} // namespace eclipse
