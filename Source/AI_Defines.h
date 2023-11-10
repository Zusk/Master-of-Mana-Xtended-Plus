#pragma once

#ifndef AI_DEFINES_H
#define AI_DEFINES_H

#define DEFAULT_PLAYER_CLOSENESS 6
#define AI_DAGGER_THRESHOLD			100  //higher is a lower chance

#define AI_DEFAULT_STRATEGY             (1 << 0)
#define AI_STRATEGY_DAGGER              (1 << 1)
#define AI_STRATEGY_SLEDGEHAMMER        (1 << 2)
#define AI_STRATEGY_CASTLE              (1 << 3)
#define AI_STRATEGY_FASTMOVERS          (1 << 4)
#define AI_STRATEGY_SLOWMOVERS          (1 << 5)
#define AI_STRATEGY_CULTURE1            (1 << 6)  //religions and wonders
#define AI_STRATEGY_CULTURE2            (1 << 7)  //mass culture buildings
#define AI_STRATEGY_CULTURE3            (1 << 8)  //culture slider
#define AI_STRATEGY_CULTURE4			(1 << 9)
#define AI_STRATEGY_MISSIONARY          (1 << 10)
#define AI_STRATEGY_CRUSH				(1 << 11)  //convert units to City Attack
#define AI_STRATEGY_PRODUCTION          (1 << 12)
#define AI_STRATEGY_PEACE				(1 << 13)  //lucky... neglect defenses.
#define AI_STRATEGY_GET_BETTER_UNITS	(1 << 14)
#define AI_STRATEGY_LAND_BLITZ			(1 << 15)
#define AI_STRATEGY_AIR_BLITZ			(1 << 16)
#define AI_STRATEGY_LAST_STAND			(1 << 17)
#define AI_STRATEGY_FINAL_WAR			(1 << 18)
#define AI_STRATEGY_OWABWNW				(1 << 19)
#define AI_STRATEGY_BIG_ESPIONAGE		(1 << 20)


#define AI_CITY_ROLE_VALID              (1 <<  1)    //zero is bad
#define AI_CITY_ROLE_BIG_CULTURE        (1 <<  2)    //culture victory, probably
#define AI_CITY_ROLE_BIG_PRODUCTION     (1 <<  3)    //don't build girly NW's
#define AI_CITY_ROLE_BIG_MILITARY       (1 <<  4)    //stick with military stuff
#define AI_CITY_ROLE_SCIENCE            (1 <<  5)    //
#define AI_CITY_ROLE_GOLD               (1 <<  6)    //
#define AI_CITY_ROLE_PRODUCTION         (1 <<  7)    //
#define AI_CITY_ROLE_SPECIALIST         (1 <<  8)    //
#define AI_CITY_ROLE_FISHING            (1 <<  9)   //
#define AI_CITY_ROLE_STAGING            (1 << 10)    //send troops here
#define AI_CITY_ROLE_LICHPIN            (1 << 11)    //this city must not fall

#define NO_AIGROUP_STATUS				(0)
#define STATUS_CITY_INVASION_TAKECITY (1)
#define STATUS_CITY_INVASION_WAIT_MORE_UNITS (2)
#define STATUS_CITY_INVASION_TRAIN_HERO		(3)
#define STATUS_CITY_INVASION_PILLAGE_STUFF		(4)
#define STATUS_CITY_INVASION_LOOK_FOR_OTHER_TARGET		(5)
#define STATUS_CITY_INVASION_WAIT_FOR_PEACE (6)

#define STATUS_NAVAL_IN_TRANSIT			(10)

#define NO_STRATEGY_SIEGE				(0)
#define STRATEGY_SIEGE_NORMAL			(1)			//build catapults etc.
#define STRATEGY_SIEGE_UU				(2)			//use longbows, golems, etc.
#define STRATEGY_SIEGE_FOL				(3)			//use FoL Treants
#define STRATEGY_SIEGE_MAGIC			(4)			//use Magic

#define NO_STRATEGY_MAGIC				(0)
#define STRATEGY_MAGIC_SUMMON			(1)
#define STRATEGY_MAGIC_EVOCATION		(2)
#define STRATEGY_MAGIC_GLOBAL			(3)

#define AI_TOWER_OF_MASTERY_NONE			(0)
#define AI_TOWER_OF_MASTERY_PICK_SAGE_CITIES (1)
#define AI_TOWER_OF_MASTERY_LESSER_TOWERS	(2)
#define AI_TOWER_OF_MASTERY_FINAL			(3)

#endif // AI_DEFINES_H
