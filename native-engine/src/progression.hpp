#pragma once
// ECLIPSE stat progression: seven stats, levelled separately for each job group. The protagonist
// grows by eating memories: every fallen enemy leaves one, consumed at the memory altar.
// Pure C++17 (no SDL) so it can be unit-tested headlessly.
// All numbers are static design estimates and have NOT been playtested.
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdio>
#include <initializer_list>
#include <vector>
#include <string>

namespace eclipse {

enum class Stat {
    Attack,      // +2.5 % damage per level on direct hits (not on bleed/burn/field ticks)
    Vitality,    // +5 max health per level
    Guard,       // flat damage reduction, (level+1)/2
    Precision,   // +1.5 % critical chance per level, capped at 30 %
    Ferocity,    // critical damage: 150 % + 4 % per level
    Swiftness,   // +0.75 % movement speed per level
    Haste,       // +1 % cooldown recovery speed per level (skills and ultimates only)
    Count
};
constexpr int statCount=static_cast<int>(Stat::Count);
constexpr int maxStatLevel=20;
constexpr int progressionJobs=3;                 // job ids 1 (Space Ruler), 2 (Flash Executioner), 3 (blade)
using StatLevels=std::array<int,statCount>;

// What Combat applies for the active job. Defaults are exactly "no progression".
struct StatBlock {
    float damageMul=1.f;
    int bonusHealth=0,bonusDefense=0;
    float critChance=0.f,critMul=1.5f;
    float moveMul=1.f,cooldownRate=1.f;
};

struct Progression {
    std::array<StatLevels,progressionJobs> levels{};
    int memories=0;
    unsigned revision=0;     // bumps on every change, so the host knows when to save

    static int jobIndex(int job){return job>=1&&job<=progressionJobs?job-1:-1;}
    int level(int job,Stat stat) const {
        const int j=jobIndex(job);
        return j<0?0:levels[j][static_cast<int>(stat)];
    }
    // Cost of going from `level` to `level+1`; 0 once maxed. 10, 13, 16, 21 ... about 1100 at 19 -> 20.
    static int upgradeCost(int level){
        if(level<0)level=0;
        if(level>=maxStatLevel)return 0;
        return static_cast<int>(10.0*std::pow(1.28,level)+.5);
    }
    static StatBlock block(const StatLevels& l){
        auto lv=[&](Stat s){return std::clamp(l[static_cast<int>(s)],0,maxStatLevel);};
        StatBlock b;
        b.damageMul=1.f+.025f*lv(Stat::Attack);
        b.bonusHealth=5*lv(Stat::Vitality);
        b.bonusDefense=(lv(Stat::Guard)+1)/2;
        b.critChance=std::min(.30f,.015f*lv(Stat::Precision));
        b.critMul=1.5f+.04f*lv(Stat::Ferocity);
        b.moveMul=1.f+.0075f*lv(Stat::Swiftness);
        b.cooldownRate=1.f+.01f*lv(Stat::Haste);
        return b;
    }
    StatBlock blockFor(int job) const {
        const int j=jobIndex(job);
        return j<0?StatBlock{}:block(levels[j]);
    }
    int nextCost(int job,Stat stat) const {return upgradeCost(level(job,stat));}
    bool canUpgrade(int job,Stat stat) const {
        if(jobIndex(job)<0)return false;
        const int cost=nextCost(job,stat);
        return cost>0&&memories>=cost;
    }
    bool upgrade(int job,Stat stat){
        if(!canUpgrade(job,stat))return false;
        memories-=nextCost(job,stat);
        ++levels[jobIndex(job)][static_cast<int>(stat)];
        ++revision;
        return true;
    }
    void addMemories(int amount){if(amount>0){memories+=amount;++revision;}}
    // Memories left by one fallen enemy: about 1 per 40 max health, never less than 1; bosses leave 5x.
    static int memoryDrop(int maxHealth,bool boss){return std::max(1,maxHealth/40)*(boss?5:1);}
    // Damage after the attack stat and a critical roll. `roll` is in [0,1); a roll below the crit
    // chance crits. Deterministic so tests do not depend on randomness. No stats -> unchanged.
    static int applyDamage(int amount,const StatBlock& b,float roll,bool* crit=nullptr){
        const bool isCrit=roll<b.critChance;
        if(crit)*crit=isCrit;
        if(amount<=0)return amount;
        float value=amount*b.damageMul;
        if(isCrit)value*=b.critMul;
        return std::max(1,static_cast<int>(value+.5f));
    }

    // ---- persistence: one small text file, written by the host ----------------------
    // ECLIPSE_PROGRESS 1 memories N   (files written before the rename say "shards"; both load)
    // job 1 l0 l1 l2 l3 l4 l5 l6   (one line per job)
    bool load(const std::string& path){
        FILE* f=std::fopen(path.c_str(),"r");if(!f)return false;
        Progression loaded;int version=0,savedMemories=0;char word[16]={};bool ok=false;
        if(std::fscanf(f,"ECLIPSE_PROGRESS %d %15s %d",&version,word,&savedMemories)==3&&version==1&&
           (std::string(word)=="memories"||std::string(word)=="shards")){
            ok=true;loaded.memories=std::max(0,savedMemories);
            for(int line=0;line<progressionJobs;++line){
                int job=0;
                if(std::fscanf(f," job %d",&job)!=1||jobIndex(job)<0){ok=false;break;}
                for(int s=0;s<statCount;++s){
                    int value=0;
                    if(std::fscanf(f," %d",&value)!=1){ok=false;break;}
                    loaded.levels[jobIndex(job)][s]=std::clamp(value,0,maxStatLevel);
                }
                if(!ok)break;
            }
        }
        std::fclose(f);
        if(!ok)return false;                 // a damaged file leaves the current state untouched
        const unsigned rev=revision;
        *this=loaded;revision=rev+1;
        return true;
    }
    bool save(const std::string& path) const {
        if(path.empty())return false;
        FILE* f=std::fopen(path.c_str(),"w");if(!f)return false;
        std::fprintf(f,"ECLIPSE_PROGRESS 1 memories %d\n",memories);
        for(int j=0;j<progressionJobs;++j){
            std::fprintf(f,"job %d",j+1);
            for(int s=0;s<statCount;++s)std::fprintf(f," %d",levels[j][s]);
            std::fprintf(f,"\n");
        }
        const bool ok=std::fflush(f)==0;
        std::fclose(f);
        return ok;
    }
};

// ---- Run upgrades (회차 강화): a skill tree per job, fed by job mastery. -----------------------
// Playing a job earns that job's mastery experience (one point of xp per fallen foe, five per boss).
// Mastery points are earned from xp and spent on the job's tree. Everything resets when the run is lost.
//
// The tree is DATA: each job has a list of nodes (`masteryTree(job)`); a node has a position on a free
// grid, a list of parents and how many of them must be owned (`need`), a point cost, stat/skill
// MODIFIERS (`Mod`, applied additively or multiplicatively) and special-effect PROCS (`ProcDef`: "when X
// happens, maybe do Y"). Growing a tree means appending nodes; nothing else in the code has to change,
// unless a brand-new Mod or ProcAction is invented (add it to the enum and its one switch case).
constexpr int maxMasteryNodes=64;          // per job (the owned-flag array; raise it if a tree outgrows it)
constexpr int maxNodeParents=4,maxNodeMods=4,maxNodeProcs=2;

// ---- modifiers: numeric/skill changes, folded into MasteryBlock -------------------------------
enum class Mod {
    // Space Ruler skills
    BoltMul,TwinBolt,FieldDurationMul,SpikesMul,RepulseRadiusMul,SingularityMul,SingularityCooldownMul,
    // Flash Executioner skills
    LanceSpeedBonus,LanceCapBonus,LanceMul,VerdictCooldownMul,RushMul,BlinkCooldownMul,BlinkInvulnerability,
    // Blade skills
    ComboMul,BleedStacksBonus,ThrustMul,SweepMul,SweepReachMul,HealPerKill,LastStandMul,
    // Raw stats: join the permanent stats while the job is active
    RawDamage,RawHealth,RawDefense,RawCrit,RawMove,RawHaste,
    Count
};
struct ModEffect {Mod mod;float value;};

// ---- procs: special effects that fire on a trigger -----------------------------------------
enum class Trigger {OnKill,OnCrit,OnDash,OnDamaged,OnSkill,Count};
enum class ProcAction {
    Explode,         // damage `value` to foes within `radius` of the target (or of the player if there is none)
    Pulse,           // damage `value` and knock back foes within `radius` of the player
    Bleed,           // `value` bleed stacks on the target
    AddLance,        // `value` stored light lances (Flash Executioner)
    Heal,            // `value` HP
    RefundCooldown,  // `value` seconds off every skill and ultimate cooldown
    Count
};
struct ProcDef {
    Trigger trigger=Trigger::OnKill;ProcAction action=ProcAction::Heal;
    float chance=1.f;     // 0..1 per trigger
    float value=0.f,radius=0.f;
    float cooldown=0.f;   // seconds before it can fire again
    int every=1;          // fires on every Nth trigger (1 = each one)
};

struct MasteryBlock {
    // Signature: Space Ruler
    float boltMul=1.f;bool twinBolt=false;float fieldDurationMul=1.f,spikesMul=1.f,repulseRadiusMul=1.f;
    float singularityMul=1.f,singularityCooldownMul=1.f;
    // Signature: Flash Executioner
    float lanceSpeedBonus=0.f;int lanceCapBonus=0;float lanceMul=1.f,verdictCooldownMul=1.f,rushMul=1.f;
    float blinkCooldownMul=1.f,blinkInvulnerability=0.f;
    // Signature: Blade
    float comboMul=1.f;int bleedStacksBonus=0;float thrustMul=1.f,sweepMul=1.f,sweepReachMul=1.f;
    int healPerKill=0;float lastStandMul=1.f;
    // Raw job stats: added to the permanent stats while this job is active (see Combat::refreshStats).
    float rawDamage=0.f;int rawHealth=0,rawDefense=0;float rawCrit=0.f,rawMove=0.f,rawHaste=0.f;
    // Special effects of every owned node.
    std::vector<ProcDef> procs;

    // One place that knows what each Mod does: multipliers multiply, flags set, everything else adds.
    void apply(Mod mod,float v){
        switch(mod){
        case Mod::BoltMul:boltMul*=v;break;
        case Mod::TwinBolt:twinBolt=v!=0.f;break;
        case Mod::FieldDurationMul:fieldDurationMul*=v;break;
        case Mod::SpikesMul:spikesMul*=v;break;
        case Mod::RepulseRadiusMul:repulseRadiusMul*=v;break;
        case Mod::SingularityMul:singularityMul*=v;break;
        case Mod::SingularityCooldownMul:singularityCooldownMul*=v;break;
        case Mod::LanceSpeedBonus:lanceSpeedBonus+=v;break;
        case Mod::LanceCapBonus:lanceCapBonus+=static_cast<int>(v);break;
        case Mod::LanceMul:lanceMul*=v;break;
        case Mod::VerdictCooldownMul:verdictCooldownMul*=v;break;
        case Mod::RushMul:rushMul*=v;break;
        case Mod::BlinkCooldownMul:blinkCooldownMul*=v;break;
        case Mod::BlinkInvulnerability:blinkInvulnerability+=v;break;
        case Mod::ComboMul:comboMul*=v;break;
        case Mod::BleedStacksBonus:bleedStacksBonus+=static_cast<int>(v);break;
        case Mod::ThrustMul:thrustMul*=v;break;
        case Mod::SweepMul:sweepMul*=v;break;
        case Mod::SweepReachMul:sweepReachMul*=v;break;
        case Mod::HealPerKill:healPerKill+=static_cast<int>(v);break;
        case Mod::LastStandMul:lastStandMul*=v;break;
        case Mod::RawDamage:rawDamage+=v;break;
        case Mod::RawHealth:rawHealth+=static_cast<int>(v);break;
        case Mod::RawDefense:rawDefense+=static_cast<int>(v);break;
        case Mod::RawCrit:rawCrit+=v;break;
        case Mod::RawMove:rawMove+=v;break;
        case Mod::RawHaste:rawHaste+=v;break;
        case Mod::Count:break;
        }
    }
};

// ---- node definitions ----------------------------------------------------------------------
struct MasteryNodeDef {
    const char* name="";      // at most 11 characters (it must fit the node box)
    const char* tag="";       // one-line value, at most 16 characters
    const char* desc="";      // one line for the detail box, at most 44 characters
    bool raw=false;           // true: flat stat node, false: signature/special node (only changes the marker)
    int cost=1;               // mastery points
    float x=0,y=0;            // free grid position: 1 unit is one node column / row; x=0 is the tree centre
    int parents[maxNodeParents]={-1,-1,-1,-1};int parentCount=0;
    int need=0;               // how many of the parents must be owned (0 = always open)
    ModEffect mods[maxNodeMods]={};int modCount=0;
    ProcDef procs[maxNodeProcs]={};int procCount=0;
};

// Compact builder so tree tables stay one line per node.
inline MasteryNodeDef masteryNode(const char* name,const char* tag,bool raw,int cost,float x,float y,
                                  std::initializer_list<int> parents,int need,
                                  std::initializer_list<ModEffect> mods,
                                  std::initializer_list<ProcDef> procs={},const char* desc=""){
    MasteryNodeDef n;n.name=name;n.tag=tag;n.desc=desc;n.raw=raw;n.cost=cost;n.x=x;n.y=y;n.need=need;
    for(int p:parents)if(n.parentCount<maxNodeParents)n.parents[n.parentCount++]=p;
    for(const auto& m:mods)if(n.modCount<maxNodeMods)n.mods[n.modCount++]=m;
    for(const auto& pr:procs)if(n.procCount<maxNodeProcs)n.procs[n.procCount++]=pr;
    return n;
}
inline ProcDef masteryProc(Trigger t,ProcAction a,float value,float chance=1.f,float radius=0.f,float cooldown=0.f,int every=1){
    ProcDef d;d.trigger=t;d.action=a;d.value=value;d.chance=chance;d.radius=radius;d.cooldown=cooldown;d.every=every;return d;
}

// The three trees. The first ten nodes of each job are a pyramid (4 / 3 / 2 / 1 per tier: tier 2 needs
// BOTH of its two parents, tier 3 any TWO tier-2 nodes, tier 4 BOTH tier-3 nodes); nodes 10-12 add
// special effects and a capstone that needs two different parents. Add more nodes at the end of a list.
inline const std::vector<MasteryNodeDef>& masteryTree(int job){
    using M=Mod;using T=Trigger;using A=ProcAction;
    static const std::vector<MasteryNodeDef> none;
    static const std::vector<MasteryNodeDef> spaceRuler={
        masteryNode("DENSE CORE","BOLT +25%",false,1,-1.5f,0,{},0,{{M::BoltMul,1.25f}},{},"VOID BOLT HITS 25% HARDER"),
        masteryNode("CLEAR MIND","CRIT +6%",true,1,-.5f,0,{},0,{{M::RawCrit,.06f}},{},"+6% CRITICAL CHANCE"),
        masteryNode("DEEP WELL","FIELD 4.5S",false,1,.5f,0,{},0,{{M::FieldDurationMul,1.5f}},{},"GRAVITY FIELD LASTS 50% LONGER"),
        masteryNode("LIGHT STEP","MOVE +10%",true,1,1.5f,0,{},0,{{M::RawMove,.10f}},{},"+10% MOVEMENT SPEED"),
        masteryNode("TWIN BOLT","2ND BOLT",false,2,-1,1,{0,1},2,{{M::TwinBolt,1.f}},{},"EACH CAST FIRES A SECOND BOLT"),
        masteryNode("SPIKE STORM","SPIKES +40%",false,2,0,1,{1,2},2,{{M::SpikesMul,1.4f}},{},"DIMENSIONAL SPIKES HIT 40% HARDER"),
        masteryNode("DARK BODY","HP +30",true,2,1,1,{2,3},2,{{M::RawHealth,30.f}},{},"+30 MAXIMUM HEALTH"),
        masteryNode("WIDE PULSE","PULSE +25%",false,3,-.5f,2,{4,5,6},2,{{M::RepulseRadiusMul,1.25f}},{},"REPULSION REACHES 25% FARTHER"),
        masteryNode("FOCUS","HASTE +10%",true,3,.5f,2,{4,5,6},2,{{M::RawHaste,.10f}},{},"SKILLS RECOVER 10% FASTER"),
        masteryNode("HUNGRY DARK","VOID +50%",false,5,0,3,{7,8},2,{{M::SingularityMul,1.5f},{M::SingularityCooldownMul,.75f}},{},"SINGULARITY +50% DAMAGE, 25% FASTER"),
        masteryNode("VOID SEED","KILL: BLAST",false,3,2.3f,2,{5,6},2,{},{masteryProc(T::OnKill,A::Explode,45,.30f,130)},"30% OF KILLS BURST ON NEARBY FOES"),
        masteryNode("GRAV SNAP","HIT: PULSE",false,3,-2.3f,2,{7,8},1,{},{masteryProc(T::OnDamaged,A::Pulse,20,1.f,170,8.f)},"WHEN HURT, PUSH FOES AWAY (8S)"),
        masteryNode("COLLAPSE","3RD SKILL",false,6,0,4.2f,{9,10},2,{},{masteryProc(T::OnSkill,A::RefundCooldown,1.5f,1.f,0,0,3)},"EVERY 3RD SKILL REFUNDS 1.5S OF COOLDOWN")};
    static const std::vector<MasteryNodeDef> flash={
        masteryNode("KEEN EDGE","DMG +10%",true,1,-1.5f,0,{},0,{{M::RawDamage,.10f}},{},"+10% DIRECT-HIT DAMAGE"),
        masteryNode("ORB SPRINT","LANCE SPD +4%",false,1,-.5f,0,{},0,{{M::LanceSpeedBonus,.04f}},{},"EACH STORED LANCE ADDS +4% ATTACK SPEED"),
        masteryNode("SHARP LANCE","LANCE +30%",false,1,.5f,0,{},0,{{M::LanceMul,1.3f}},{},"LIGHT LANCES HIT 30% HARDER"),
        masteryNode("FLEET","MOVE +12%",true,1,1.5f,0,{},0,{{M::RawMove,.12f}},{},"+12% MOVEMENT SPEED"),
        masteryNode("FLOW","HASTE +10%",true,2,-1,1,{0,1},2,{{M::RawHaste,.10f}},{},"SKILLS RECOVER 10% FASTER"),
        masteryNode("OVERFLOW","LANCES +2",false,2,0,1,{1,2},2,{{M::LanceCapBonus,2.f}},{},"STORE 2 MORE LIGHT LANCES"),
        masteryNode("LONG RUSH","RUSH +40%",false,2,1,1,{2,3},2,{{M::RushMul,1.4f}},{},"RUSH HITS 40% HARDER"),
        masteryNode("SKY RAIN","VERDICT CD-33",false,3,-.5f,2,{4,5,6},2,{{M::VerdictCooldownMul,.67f}},{},"VERDICT RECOVERS 33% FASTER"),
        masteryNode("LIGHT BODY","HP +30",true,3,.5f,2,{4,5,6},2,{{M::RawHealth,30.f}},{},"+30 MAXIMUM HEALTH"),
        masteryNode("AFTERFLASH","BLINK SAFE",false,5,0,3,{7,8},2,{{M::BlinkInvulnerability,.6f},{M::BlinkCooldownMul,.67f}},{},"BLINK GRANTS 0.6S SAFETY, 33% FASTER"),
        masteryNode("LIGHT BURST","CRIT: +1 LANCE",false,3,2.3f,2,{5,6},2,{},{masteryProc(T::OnCrit,A::AddLance,1)},"EVERY CRITICAL HIT STORES A LANCE"),
        masteryNode("AFTERIMAGE","DASH: BLAST",false,3,-2.3f,2,{7,8},1,{},{masteryProc(T::OnDash,A::Explode,30,1.f,110,1.f)},"DASHING LEAVES A BLAST BEHIND"),
        masteryNode("RADIANT END","KILL: LANCE+HP",false,6,0,4.2f,{9,10},2,{},{masteryProc(T::OnKill,A::AddLance,1),masteryProc(T::OnKill,A::Heal,2)},"KILLS STORE A LANCE AND HEAL 2")};
    static const std::vector<MasteryNodeDef> blade={
        masteryNode("HEAVY HAND","COMBO +25%",false,1,-1.5f,0,{},0,{{M::ComboMul,1.25f}},{},"BASIC COMBO HITS 25% HARDER"),
        masteryNode("BRUTE","DMG +10%",true,1,-.5f,0,{},0,{{M::RawDamage,.10f}},{},"+10% DIRECT-HIT DAMAGE"),
        masteryNode("PIERCING","THRUST +40%",false,1,.5f,0,{},0,{{M::ThrustMul,1.4f}},{},"THRUST HITS 40% HARDER"),
        masteryNode("THICK SKIN","DEF +2",true,1,1.5f,0,{},0,{{M::RawDefense,2.f}},{},"TAKE 2 LESS DAMAGE FROM EVERY HIT"),
        masteryNode("BLOODY EDGE","BLEED +1",false,2,-1,1,{0,1},2,{{M::BleedStacksBonus,1.f}},{},"THE THIRD HIT BLEEDS ONE STACK MORE"),
        masteryNode("IRON BODY","HP +40",true,2,0,1,{1,2},2,{{M::RawHealth,40.f}},{},"+40 MAXIMUM HEALTH"),
        masteryNode("WIDE SWEEP","SWEEP +50%",false,2,1,1,{2,3},2,{{M::SweepMul,1.5f},{M::SweepReachMul,1.15f}},{},"SWEEP HITS 50% HARDER AND REACHES FARTHER"),
        masteryNode("SECOND WIND","HEAL 3/KILL",false,3,-.5f,2,{4,5,6},2,{{M::HealPerKill,3.f}},{},"HEAL 3 HP FOR EVERY FOE THAT FALLS"),
        masteryNode("BULWARK","DEF +3",true,3,.5f,2,{4,5,6},2,{{M::RawDefense,3.f}},{},"TAKE 3 LESS DAMAGE FROM EVERY HIT"),
        masteryNode("LAST STAND","LOW HP +30%",false,5,0,3,{7,8},2,{{M::LastStandMul,1.3f}},{},"+30% DAMAGE BELOW 40% HEALTH"),
        masteryNode("RUPTURE","CRIT: BLEED",false,3,2.3f,2,{5,6},2,{},{masteryProc(T::OnCrit,A::Bleed,2)},"CRITICAL HITS APPLY 2 BLEED STACKS"),
        masteryNode("COUNTER","HIT: BLAST",false,3,-2.3f,2,{7,8},1,{},{masteryProc(T::OnDamaged,A::Explode,60,.35f,140,3.f)},"35% WHEN HURT: BLAST AROUND YOU"),
        masteryNode("BLOODLUST","KILL: HEAL+CD",false,6,0,4.2f,{9,10},2,{},{masteryProc(T::OnKill,A::Heal,5),masteryProc(T::OnKill,A::RefundCooldown,1.f)},"KILLS HEAL 5 AND REFUND 1S OF COOLDOWN")};
    return job==1?spaceRuler:job==2?flash:job==3?blade:none;
}
inline int masteryNodeCount(int job){return static_cast<int>(masteryTree(job).size());}
inline const MasteryNodeDef* masteryNodeDef(int job,int node){
    const auto& tree=masteryTree(job);
    return node>=0&&node<static_cast<int>(tree.size())?&tree[node]:nullptr;
}
inline const char* masteryNodeName(int job,int node){const auto* d=masteryNodeDef(job,node);return d?d->name:"";}
inline const char* masteryNodeTag(int job,int node){const auto* d=masteryNodeDef(job,node);return d?d->tag:"";}
inline const char* masteryNodeDesc(int job,int node){const auto* d=masteryNodeDef(job,node);return d?d->desc:"";}
inline bool masteryNodeIsRaw(int job,int node){const auto* d=masteryNodeDef(job,node);return d&&d->raw;}

struct MasteryState {
    std::array<int,progressionJobs> xp{};
    std::array<std::array<bool,maxMasteryNodes>,progressionJobs> owned{};

    // Point k needs k^2 + 2k total xp: 3, 8, 15, 24, 35 ... so earned = floor(sqrt(xp + 1)) - 1.
    static int earnedPoints(int xp){return xp<=0?0:static_cast<int>(std::floor(std::sqrt(xp+1.0)))-1;}
    static int xpForPoint(int k){return k*k+2*k;}
    static int nodeCost(int job,int node){const auto* d=masteryNodeDef(job,node);return d?d->cost:0;}
    static int totalCost(int job){int t=0;for(int n=0;n<masteryNodeCount(job);++n)t+=nodeCost(job,n);return t;}
    int xpOf(int job) const {const int j=Progression::jobIndex(job);return j<0?0:xp[j];}
    int spent(int job) const {
        const int j=Progression::jobIndex(job);if(j<0)return 0;
        int total=0;for(int n=0;n<masteryNodeCount(job);++n)if(owned[j][n])total+=nodeCost(job,n);return total;
    }
    int points(int job) const {return std::max(0,earnedPoints(xpOf(job))-spent(job));}
    // xp still missing for the next point.
    int xpToNext(int job) const {return std::max(0,xpForPoint(earnedPoints(xpOf(job))+1)-xpOf(job));}
    bool has(int job,int node) const {
        const int j=Progression::jobIndex(job);
        return j>=0&&node>=0&&node<maxMasteryNodes&&node<masteryNodeCount(job)&&owned[j][node];
    }
    // How many parents of the node are owned, and whether that opens it.
    int parentsOwned(int job,int node) const {
        const auto* d=masteryNodeDef(job,node);if(!d)return 0;
        int count=0;
        for(int i=0;i<d->parentCount;++i)if(has(job,d->parents[i]))++count;
        return count;
    }
    bool unlocked(int job,int node) const {
        const auto* d=masteryNodeDef(job,node);if(!d)return false;
        return d->parentCount==0||parentsOwned(job,node)>=d->need;
    }
    bool canBuy(int job,int node) const {
        const int j=Progression::jobIndex(job);
        if(j<0||!masteryNodeDef(job,node)||node>=maxMasteryNodes||owned[j][node])return false;
        return unlocked(job,node)&&points(job)>=nodeCost(job,node);
    }
    bool buy(int job,int node){
        if(!canBuy(job,node))return false;
        owned[Progression::jobIndex(job)][node]=true;
        return true;
    }
    void addXp(int job,int amount){const int j=Progression::jobIndex(job);if(j>=0&&amount>0)xp[j]+=amount;}
    static int xpDrop(bool boss){return boss?5:1;}

    // Fold every owned node of the job into one block: modifiers apply, procs are collected.
    MasteryBlock block(int job) const {
        MasteryBlock b;
        const auto& tree=masteryTree(job);
        for(int n=0;n<static_cast<int>(tree.size())&&n<maxMasteryNodes;++n){
            if(!has(job,n))continue;
            for(int i=0;i<tree[n].modCount;++i)b.apply(tree[n].mods[i].mod,tree[n].mods[i].value);
            for(int i=0;i<tree[n].procCount;++i)b.procs.push_back(tree[n].procs[i]);
        }
        return b;
    }
};

} // namespace eclipse
