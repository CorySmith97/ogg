#ifndef PATHFINDER_H
#define PATHFINDER_H

typedef enum {
    BC_BARD, 
    BC_CLERIC, 
    BC_DRUID, 
    BC_FIGHTER, 
    BC_RANGER, 
    BC_ROGUE, 
    BC_WITCH, 
    BC_WIZARD,
} BaseClass;

typedef enum {
    AH_AIUVARIN,
    AH_ANCIENT_ELF,
    AH_ANCIENT_BLOODED_DWARF,
    AH_ARCTIC_ELF,
    AH_BADLANDS_ORC,
    AH_BATTLE_READY_ORC,
    AH_CACTUS_LESHY,
    AH_CAVERN_ELF,
    AH_CHAMELEON_GNOME,
    AH_CHANGELING,
    AH_CHARHIDE_GOBLIN,
    AH_DEATH_WARDEN_DWARF,
    AH_DEEP_ORC,
    AH_DROMAAR,
    AH_FEY_TOUCHED_GNOME,
    AH_FORGE_DWARF,
    AH_FRUIT_LESHY,
    AH_FUNGUS_LESHY,
    AH_GOURD_LESHY,
    AH_GRAVE_ORC,
    AH_GUTSY_HALFLING,
    AH_HILLOCK_HALFLING,
    AH_HOLD_SCARRED_ORC,
    AH_IRONGUT_GOBLIN,
    AH_JINXED_HALFLING,
    AH_LEAF_LESHY,
    AH_LOTUS_LESHY,
    AH_NEPHILIM,
    AH_NOMADIC_HALFLING,
    AH_RAINFALL_ORC,
    AH_RAZORTOOTH_GOBLIN,
    AH_ROCK_DWARF,
    AH_ROOT_LESHY,
    AH_SEAWEED_LESHY,
    AH_SEER_ELF,
    AH_SENSATE_GNOME,
    AH_SKILLED_HUMAN,
    AH_SNOW_GOBLIN,
    AH_STRONG_BLOODED_DWARF,
    AH_TWILIGHT_HALFLING,
    AH_UMBRAL_GNOME,
    AH_UNBREAKABLE_GOBLIN,
    AH_VERSATILE_HUMAN,
    AH_VINE_LESHY,
    AH_WELLSPRING_GNOME,
    AH_WHISPER_ELF,
    AH_WILDWOOD_HALFLING,
    AH_WINTER_ORC,
    AH_WOODLAND_ELF
} AncestoryHeritage;

typedef enum {
    FEAT_THIS,
} Feat;

typedef enum {
    BG_ACOLYTE, 
    BG_ACROBAT, 
    BG_ANIMAL_WHISPERER, 
    BG_ARTISAN, 
    BG_ARTIST, 
    BG_BANDIT, 
    BG_BARKEEP, 
    BG_BARRISTER, 
    BG_BOUNTY_HUNTER, 
    BG_CHARLATAN, 
    BG_COOK, 
    BG_CRIMINAL, 
    BG_CULTIST, 
    BG_DETECTIVE, 
    BG_EMISSARY, 
    BG_ENTERTAINER, 
    BG_FARMHAND, 
    BG_FIELD_MEDIC, 
    BG_FORTUNE_TELLER, 
    BG_GAMBLER, 
    BG_GLADIATOR, 
    BG_GUARD, 
    BG_HERBALIST, 
    BG_HERMIT, 
    BG_HUNTER, 
    BG_LABORER, 
    BG_MARTIAL_DISCIPLE, 
    BG_MERCHANT, 
    BG_MINER, 
    BG_NOBLE, 
    BG_NOMAD, 
    BG_PRISONER, 
    BG_RAISED_BY_BELIEF, 
    BG_SAILOR, 
    BG_SCHOLAR, 
    BG_SCOUT, 
    BG_STREET_URCHIN, 
    BG_TEACHER, 
    BG_TINKER, 
    BG_WARRIOR,
} Background;

typedef enum {
    SE_BLANK,
} StatusEffect;

typedef struct {
    u32 faces;
    u32 count;
} Dice;

typedef struct {
    u16 strength;
    u16 dexterity;
    u16 constitution;
    u16 intelligence;
    u16 wisdom;
    u16 charisma;
} Attributes;

typedef enum {
    DT_BLUDGENING,
    DT_PIERCING,
} DamageType;

typedef enum Skill {
    EATING,
} Skill;

typedef enum SpellName {
    SPELL_ARCANE_TOUCH
} SpellName;

typedef union Action {
    struct {
        String8 name;
        String8 desc;
        SpellName name_type;
        Dice damage;
    } Spell;
} Action;


void roll_dice(Dice d);

#endif // PATHFINDER_H
