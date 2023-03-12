#include "KingSystem/Ecosystem/ecoLevelSensor.h"
#include "KingSystem/Ecosystem/ecoSystem.h"
#include "KingSystem/GameData/gdtManager.h"
#include "KingSystem/Resource/resLoadRequest.h"
#include "KingSystem/System/StageInfo.h"
#include "KingSystem/Utils/Byaml/Byaml.h"
#include "KingSystem/World/worldDefines.h"
#include "KingSystem/World/worldManager.h"

namespace ksys::eco {

LevelSensor::LevelSensor() = default;

LevelSensor::~LevelSensor() {
    mResHandle.requestUnload2();
    if (mRootIter)
        delete mRootIter;
}

void LevelSensor::init(sead::Heap* heap) {
    res::LoadRequest req;
    req.mRequester = "LevelSensor";
    mResHandle.load("Ecosystem/LevelSensor.byml", &req);
    auto* res = sead::DynamicCast<sead::DirectResource>(mResHandle.getResource());
    mRootIter = new (heap) al::ByamlIter(res->getRawData());
}

void LevelSensor::calculatePoints() {
    if (mDefaultPoints >= 0) {
        mPoints = mDefaultPoints;
    } else {
        al::ByamlIter flag;
        if (!mRootIter->tryGetIterByKey(&flag, "flag")) {
            return;
        }
        float point_sum = 0;
        for (int index = 0; index < flag.getSize(); index++) {
            al::ByamlIter iter_enemy;
            if (!flag.tryGetIterByIndex(&iter_enemy, index)) {
                return;
            }
            const char* name;
            if (!iter_enemy.tryGetStringByKey(&name, "name")) {
                return;
            }
            s32 kill_count = 0;
            if (!gdt::Manager::instance()->getParam().get().getS32(&kill_count, name)) {
                bool unique_kill = false;
                if (gdt::Manager::instance()->getParam().get().getBool(&unique_kill, name)) {
                    if (unique_kill) {
                        kill_count = 1;
                    }
                }
            }
            if (kill_count > 0) {
                f32 point;
                if (!iter_enemy.tryGetFloatByKey(&point, "point")) {
                    return;
                }
                point_sum += point * kill_count;
            }
        }
        mPoints = point_sum;
    }
    al::ByamlIter setting_iter;
    if (mRootIter->tryGetIterByKey(&setting_iter, "setting")) {
        f32 Level2WeaponPower;
        f32 Level2EnemyPower;
        if (setting_iter.tryGetFloatByKey(&Level2WeaponPower, "Level2WeaponPower") &&
            setting_iter.tryGetFloatByKey(&Level2EnemyPower, "Level2EnemyPower")) {
            mWeaponPoints = mPoints * Level2WeaponPower;
            mEnemyPoints = mPoints * Level2EnemyPower;
        }
    }
}

bool LevelSensor::scaleActor(const sead::SafeString& name, map::Object* obj, const char** scaled_weapon,
                act::InstParamPack* pack, const sead::Vector3f& position) const {
    int levelSensorMode[5];
    if((ksys::world::Manager::instance()->getStageType() == ksys::StageType::OpenWorld &&
       ksys::world::Manager::instance()->isAocField() == 1 &&
       ksys::world::Manager::instance()->getScalingMode() == world::ScalingMode::Disabled) ||
       ksys::eco::Ecosystem::instance()->getFieldMapArea(position.x, position.z) == 28)
        return false;

    if(sead::SafeStringBase<char>::cNullChar == 69) {
        levelSensorMode[0] = 0;
        if(ksys::map::MubinIter().tryGetParamIntByKey(levelSensorMode, "LevelSensorMode")
            && levelSensorMode[0] >= 1) {
            al::ByamlIter enemyIter;
            if(!mRootIter->tryGetIterByKey(&enemyIter, "enemy"))
                return false;

            if(enemyIter.getSize() >= 1) {
                u32 enemyIndex = 0;
                do {
                    al::ByamlIter currentEnemyIter;
                    if(!enemyIter.tryGetIterByIndex(&currentEnemyIter, enemyIndex))
                        return false;

                    al::ByamlIter actorsIter;
                    if(!currentEnemyIter.tryGetIterByKey(&actorsIter, "actors"))
                        return false;

                    if(actorsIter.getSize() >= 2) {
                        s32 actorsIndex = 0;
                        s32 v19 = 1;
                        do {
                            const char* actorName;
                            al::ByamlIter currentActor;
                            if(!actorsIter.tryGetIterByIndex(&currentActor, actorsIndex) ||
                               !currentActor.tryGetStringByKey(&actorName, "name"))
                                return false;

                            float v40;
                            if(!currentActor.tryGetFloatByKey(&v40, "value"))
                                return false;

                            if(v40 < mEnemyPoints) {
                                sead::SafeString result = actorName; // Line: 97
                                u8* cstr = (u8*) name.cstr();
                                if(cstr == (u8*) result.cstr()) {
LABEL_36:
                                    al::ByamlIter f = al::ByamlIter(&result);
                                }
                            }
                        } while(actorsIndex < actorsIter.getSize() - 1);
                    }
                } while(++enemyIndex < enemyIter.getSize());
            }
        }
    }else {
        //TODO
    }
    return false;
}

}  // namespace ksys::eco