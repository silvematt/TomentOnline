#include <stdio.h>
#include "R_Rendering.h"
#include "A_Application.h"
#include "D_AssetsManager.h"
#include "U_Utilities.h"
#include "D_ObjectsCallbacks.h"

tomentdatapack_t tomentdatapack;

//-------------------------------------
// Sets defauls for an object
//-------------------------------------
void D_InitObject(object_t* obj)
{
    obj->flags = 0;
    obj->animations = NULL;
}

static void D_InitializeObjectAnim(objectanimations_t* obj)
{
    obj->belongsTo = NULL;
    
    obj->animIdle = NULL;
    obj->animIdleSheetLength = 0;
    obj->animIdleActionFrame = 0;
    obj->animIdleSpeedModifier = 0;

    obj->animDie = NULL;
    obj->animDieSheetLength = 0;
    obj->animDieActionFrame = 0;
    obj->animDieSpeedModifier = 0;

    obj->animAttack = NULL;
    obj->animAttackSheetLength = 0;
    obj->animAttackActionFrame = 0;
    obj->animAttackSpeedModifier = 0;

    obj->animCastSpell = NULL;
    obj->animCastSpellSheetLength = 0;
    obj->animCastSpellActionFrame = 0;
    obj->animCastSpellkSpeedModifier = 0;

    obj->animSpecial1 = NULL;
    obj->animSpecial1SheetLength = 0;
    obj->animSpecial1ActionFrame = 0;
    obj->animSpecial1SpeedModifier = 0;

    obj->animSpecial2 = NULL;
    obj->animSpecial2SheetLength = 0;
    obj->animSpecial2ActionFrame = 0;
    obj->animSpecial2SpeedModifier = 0;

    obj->animSpecial3 = NULL;
    obj->animSpecial3SheetLength = 0;
    obj->animSpecial3ActionFrame = 0;
    obj->animSpecial3SpeedModifier = 0;
}


//-------------------------------------
// Sets defauls for an object
//-------------------------------------
void D_InitWallAsset(wallAsset_t* obj)
{
    obj->flags = 0;
}

//-------------------------------------
// Sets defauls for an object
//-------------------------------------
void D_InitTextureAsset(textureObject_t* obj)
{
    obj->flags = 0;
}

//-------------------------------------
// Sets an Object 
//-------------------------------------
void D_SetObject(object_t* obj, int id, SDL_Surface* texture, object_t* alt)
{
    obj->ID = id;
    
    if(texture == NULL)
        printf("FATAL ERROR! Setting Object: Could not load texture with ID: %d\n", id);

    obj->texture = texture;
    obj->alt = alt;
}

bool D_CheckTextureLoaded(SDL_Surface* ptr, int ID)
{
    if(ptr == NULL)
    {
        printf("ERROR! Could not load IMG_ID:\"%d\", the file could not exist or could be corrupted. Attempting to fall back to Engines Defaults...\n", ID);
        return false;
    }

    return true;
}

//-------------------------------------
// Opens the archives to allow objects initializations
//-------------------------------------
void D_OpenArchs(void)
{
    archt_t* curArch;

    //-------------------------------------
    // Open IMG Arch
    //-------------------------------------
    curArch = &tomentdatapack.IMGArch;
    curArch->file = fopen("Data/img.archt", "rb");

    // Get file lenght
    fseek(curArch->file, 0, SEEK_END);
    curArch->fileLength = ftell(curArch->file);
    rewind(curArch->file);

    // Read TOC size in bytes
    // First 4 bytes are TOC length
    fread(&(curArch->tocSize), sizeof(curArch->tocSize), 1, curArch->file);

    // Get number of elements
    curArch->tocElementsLenght = curArch->tocSize / sizeof(tocElement_t);

    // Calculate base offset
    curArch->tocOffset = sizeof(curArch->tocSize) + sizeof(tocElement_t) * tomentdatapack.IMGArch.tocElementsLenght;

    // Fill the ToC
    for(int i = 0; i < curArch->tocElementsLenght; i++)
    {
        fread(&curArch->toc[i].id, sizeof(curArch->toc[i].id), 1, curArch->file);
        fread(&curArch->toc[i].startingOffset, sizeof(curArch->toc[i].startingOffset), 1, curArch->file);
        fread(&curArch->toc[i].size, sizeof(curArch->toc[i].size), 1, curArch->file);
    }
    rewind(curArch->file);

    // Allocate the buffer to allow reading the file and fill it
    curArch->buffer = (byte *)malloc((curArch->fileLength) * sizeof(byte));
    fread(tomentdatapack.IMGArch.buffer, (tomentdatapack.IMGArch.fileLength), 1, tomentdatapack.IMGArch.file);
}



//-------------------------------------
// Initializes the Assets and datapacks 
//-------------------------------------
void D_InitAssetManager(void)
{
    printf("Initializing Assets Manager...\n");

    D_OpenArchs();

    D_InitUIAssets();
    D_InitFontSheets();

    D_InitEnginesDefaults();
    D_InitLoadTextures();
    D_InitLoadWalls();
    D_InitLoadSprites();
    D_InitLoadSkies();

    D_InitLoadPlayersFP();

    D_CloseArchs();
}

void D_InitUIAssets(void)
{    
    uiAssets_t* selectCursor = (uiAssets_t*)malloc(sizeof(uiAssets_t));
    uiAssets_t* menuTitle = (uiAssets_t*)malloc(sizeof(uiAssets_t));
    uiAssets_t* healthBarEmpty = (uiAssets_t*)malloc(sizeof(uiAssets_t));
    uiAssets_t* healthBarFill = (uiAssets_t*)malloc(sizeof(uiAssets_t));
    uiAssets_t* manaBarEmpty = (uiAssets_t*)malloc(sizeof(uiAssets_t));
    uiAssets_t* manaBarFill = (uiAssets_t*)malloc(sizeof(uiAssets_t));
    uiAssets_t* iconFists = (uiAssets_t*)malloc(sizeof(uiAssets_t));
    uiAssets_t* iconAxe = (uiAssets_t*)malloc(sizeof(uiAssets_t));
    uiAssets_t* iconSpellFireball1 = (uiAssets_t*)malloc(sizeof(uiAssets_t));
    uiAssets_t* crosshair = (uiAssets_t*)malloc(sizeof(uiAssets_t));
    uiAssets_t* iconSpellIceDart = (uiAssets_t*)malloc(sizeof(uiAssets_t));
    uiAssets_t* crosshairHit = (uiAssets_t*)malloc(sizeof(uiAssets_t));
    uiAssets_t* bossHealthBarEmpty = (uiAssets_t*)malloc(sizeof(uiAssets_t));
    uiAssets_t* bossHealthBarFill = (uiAssets_t*)malloc(sizeof(uiAssets_t));
    uiAssets_t* iconGreatsword = (uiAssets_t*)malloc(sizeof(uiAssets_t));
    uiAssets_t* tankClassPotrait = (uiAssets_t*)malloc(sizeof(uiAssets_t));
    uiAssets_t* healerClassPotrait = (uiAssets_t*)malloc(sizeof(uiAssets_t));
    uiAssets_t* dpsClassPotrait = (uiAssets_t*)malloc(sizeof(uiAssets_t));
    uiAssets_t* tankClassIcon = (uiAssets_t*)malloc(sizeof(uiAssets_t));
    uiAssets_t* healerClassIcon = (uiAssets_t*)malloc(sizeof(uiAssets_t));
    uiAssets_t* dpsClassIcon = (uiAssets_t*)malloc(sizeof(uiAssets_t));
    uiAssets_t* classIconSelection = (uiAssets_t*)malloc(sizeof(uiAssets_t));
    uiAssets_t* classIconSelectionDisabled = (uiAssets_t*)malloc(sizeof(uiAssets_t));
    uiAssets_t* iconReady = (uiAssets_t*)malloc(sizeof(uiAssets_t));
    uiAssets_t* iconNotReady = (uiAssets_t*)malloc(sizeof(uiAssets_t));
    uiAssets_t* iconMace = (uiAssets_t*)malloc(sizeof(uiAssets_t));
    uiAssets_t* skillIconTankShieldSlam = (uiAssets_t*)malloc(sizeof(uiAssets_t));
    uiAssets_t* skillIconTankShieldBlock = (uiAssets_t*)malloc(sizeof(uiAssets_t));
    uiAssets_t* skillIconTankConsacration = (uiAssets_t*)malloc(sizeof(uiAssets_t));
    uiAssets_t* skillIconEmpty = (uiAssets_t*)malloc(sizeof(uiAssets_t));
    uiAssets_t* skillIconHealerSelfHeal = (uiAssets_t*)malloc(sizeof(uiAssets_t));
    uiAssets_t* skillIconHealerConcentratedHeal = (uiAssets_t*)malloc(sizeof(uiAssets_t));
    uiAssets_t* skillIconHealerDivineIntervention = (uiAssets_t*)malloc(sizeof(uiAssets_t));
    uiAssets_t* skillIconDpsCheapshot = (uiAssets_t*)malloc(sizeof(uiAssets_t));
    uiAssets_t* skillIconDpsObliterate = (uiAssets_t*)malloc(sizeof(uiAssets_t));
    uiAssets_t* skillIconDpsSplit = (uiAssets_t*)malloc(sizeof(uiAssets_t));
    uiAssets_t* inputfield01 = (uiAssets_t*)malloc(sizeof(uiAssets_t));
    uiAssets_t* inputfield01_active = (uiAssets_t*)malloc(sizeof(uiAssets_t));

    tomentdatapack.uiAssets[M_ASSET_SELECT_CURSOR] = selectCursor;
    tomentdatapack.uiAssets[M_ASSET_TITLE] = menuTitle;
    tomentdatapack.uiAssets[G_ASSET_HEALTHBAR_EMPTY] = healthBarEmpty;
    tomentdatapack.uiAssets[G_ASSET_HEALTHBAR_FILL] = healthBarFill;
    tomentdatapack.uiAssets[G_ASSET_MANABAR_EMPTY] = manaBarEmpty;
    tomentdatapack.uiAssets[G_ASSET_MANABAR_FILL] = manaBarFill;
    tomentdatapack.uiAssets[G_ASSET_ICON_FISTS] = iconFists;
    tomentdatapack.uiAssets[G_ASSET_ICON_AXE] = iconAxe;
    tomentdatapack.uiAssets[G_ASSET_ICON_SPELL_FIREBALL1] = iconSpellFireball1;
    tomentdatapack.uiAssets[G_ASSET_UI_CROSSHAIR] = crosshair;
    tomentdatapack.uiAssets[G_ASSET_ICON_SPELL_ICEDART1] = iconSpellIceDart;
    tomentdatapack.uiAssets[G_ASSET_UI_CROSSHAIR_HIT] = crosshairHit;
    tomentdatapack.uiAssets[G_ASSET_BOSS_HEALTHBAR_EMPTY] = bossHealthBarEmpty;
    tomentdatapack.uiAssets[G_ASSET_BOSS_HEALTHBAR_FILL] = bossHealthBarFill;
    tomentdatapack.uiAssets[G_ASSET_ICON_GREATSWORD] = iconGreatsword;
    tomentdatapack.uiAssets[G_ASSET_CLASS_POTRAIT_TANK] = tankClassPotrait;
    tomentdatapack.uiAssets[G_ASSET_CLASS_POTRAIT_HEALER] = healerClassPotrait;
    tomentdatapack.uiAssets[G_ASSET_CLASS_POTRAIT_DPS] = dpsClassPotrait;
    tomentdatapack.uiAssets[G_ASSET_CLASS_ICON_TANK] = tankClassIcon;
    tomentdatapack.uiAssets[G_ASSET_CLASS_ICON_HEALER] = healerClassIcon;
    tomentdatapack.uiAssets[G_ASSET_CLASS_ICON_DPS] = dpsClassIcon;
    tomentdatapack.uiAssets[G_ASSET_CLASS_ICON_SELECTION] = classIconSelection;
    tomentdatapack.uiAssets[G_ASSET_CLASS_ICON_SELECTION_DISABLED] = classIconSelectionDisabled;
    tomentdatapack.uiAssets[G_ASSET_ICON_READY] = iconReady;
    tomentdatapack.uiAssets[G_ASSET_ICON_NOTREADY] = iconNotReady;
    tomentdatapack.uiAssets[G_ASSET_ICON_MACE] = iconMace;
    tomentdatapack.uiAssets[G_ASSET_SKILL_ICON_TANK_SHIELD_SLAM] = skillIconTankShieldSlam;
    tomentdatapack.uiAssets[G_ASSET_SKILL_ICON_TANK_SHIELD_BLOCK] = skillIconTankShieldBlock;
    tomentdatapack.uiAssets[G_ASSET_SKILL_ICON_TANK_CONSACRATION] = skillIconTankConsacration;
    tomentdatapack.uiAssets[G_ASSET_SKILL_ICON_EMPTY] = skillIconEmpty;
    tomentdatapack.uiAssets[G_ASSET_SKILL_ICON_HEALER_SELF_HEAL] = skillIconHealerSelfHeal;
    tomentdatapack.uiAssets[G_ASSET_SKILL_ICON_HEALER_CONCENTRATED_HEAL] = skillIconHealerConcentratedHeal;
    tomentdatapack.uiAssets[G_ASSET_SKILL_ICON_HEALER_DIVINE_INTERVENTION] = skillIconHealerDivineIntervention;
    tomentdatapack.uiAssets[G_ASSET_SKILL_ICON_DPS_CHEAPSHOT] = skillIconDpsCheapshot;
    tomentdatapack.uiAssets[G_ASSET_SKILL_ICON_DPS_OBLITERATE] = skillIconDpsObliterate;
    tomentdatapack.uiAssets[G_ASSET_SKILL_ICON_DPS_SPLIT] = skillIconDpsSplit;
    tomentdatapack.uiAssets[M_ASSET_INPUTFIELD_01] = inputfield01;
    tomentdatapack.uiAssets[M_ASSET_INPUTFIELD_01_ACTIVE] = inputfield01_active;

    tomentdatapack.uiAssetsLenght = 38;

    // Fill objects
    // Convert all the surfaces that we will load in the same format as the win_surface
    SDL_Surface *temp1;     // Surface
    SDL_RWops* sdlWops;     // Structure to read bytes
    int offset;             // Offset in the img.archt

    // MENU_SELECT_CURSOR
    offset = tomentdatapack.IMGArch.tocOffset + (tomentdatapack.IMGArch.toc[IMG_ID_MENU_SELECT_CURSOR].startingOffset);
    sdlWops = SDL_RWFromConstMem((byte*)tomentdatapack.IMGArch.buffer+offset, tomentdatapack.IMGArch.toc[IMG_ID_MENU_SELECT_CURSOR].size);
    temp1 = SDL_LoadBMP_RW(sdlWops, SDL_TRUE);
    if(D_CheckTextureLoaded(temp1, IMG_ID_MENU_SELECT_CURSOR))
        tomentdatapack.uiAssets[M_ASSET_SELECT_CURSOR]->texture = SDL_ConvertSurface(temp1, win_surface->format, 0);
    else
        printf("FATAL ERROR! Engine Default \"%d\" failed to load. Further behaviour is undefined.\n", IMG_ID_EDEFAULT_1);
    SDL_SetColorKey(tomentdatapack.uiAssets[M_ASSET_SELECT_CURSOR]->texture, SDL_TRUE, r_transparencyColor);    // Make transparency color for blitting
    SDL_FreeSurface(temp1);

    // MENU_TITLE
    offset = tomentdatapack.IMGArch.tocOffset + (tomentdatapack.IMGArch.toc[IMG_ID_MENU_TITLE].startingOffset);
    sdlWops = SDL_RWFromConstMem((byte*)tomentdatapack.IMGArch.buffer+offset, tomentdatapack.IMGArch.toc[IMG_ID_MENU_TITLE].size);
    temp1 = SDL_LoadBMP_RW(sdlWops, SDL_TRUE);
    if(D_CheckTextureLoaded(temp1, IMG_ID_MENU_TITLE))
        tomentdatapack.uiAssets[M_ASSET_TITLE]->texture = SDL_ConvertSurface(temp1, win_surface->format, 0);
    else
        printf("FATAL ERROR! Engine Default \"%d\" failed to load. Further behaviour is undefined.\n", IMG_ID_EDEFAULT_1);
    SDL_SetColorKey(tomentdatapack.uiAssets[M_ASSET_TITLE]->texture, SDL_TRUE, r_transparencyColor);    // Make transparency color for blitting
    SDL_FreeSurface(temp1);

    // HEALTHBAR_EMPTY
    offset = tomentdatapack.IMGArch.tocOffset + (tomentdatapack.IMGArch.toc[IMG_ID_HEALTHBAR_EMPTY].startingOffset);
    sdlWops = SDL_RWFromConstMem((byte*)tomentdatapack.IMGArch.buffer+offset, tomentdatapack.IMGArch.toc[IMG_ID_HEALTHBAR_EMPTY].size);
    temp1 = SDL_LoadBMP_RW(sdlWops, SDL_TRUE);
    if(D_CheckTextureLoaded(temp1, IMG_ID_HEALTHBAR_EMPTY))
        tomentdatapack.uiAssets[G_ASSET_HEALTHBAR_EMPTY]->texture = SDL_ConvertSurface(temp1, win_surface->format, 0);
    else
        printf("FATAL ERROR! Engine Default \"%d\" failed to load. Further behaviour is undefined.\n", IMG_ID_EDEFAULT_1);
    SDL_SetColorKey(tomentdatapack.uiAssets[G_ASSET_HEALTHBAR_EMPTY]->texture, SDL_TRUE, r_transparencyColor);    // Make transparency color for blitting
    SDL_FreeSurface(temp1);

    // HEALTHBAR_FILL
    offset = tomentdatapack.IMGArch.tocOffset + (tomentdatapack.IMGArch.toc[IMG_ID_HEALTHBAR_FILL].startingOffset);
    sdlWops = SDL_RWFromConstMem((byte*)tomentdatapack.IMGArch.buffer+offset, tomentdatapack.IMGArch.toc[IMG_ID_HEALTHBAR_FILL].size);
    temp1 = SDL_LoadBMP_RW(sdlWops, SDL_TRUE);
    if(D_CheckTextureLoaded(temp1, IMG_ID_HEALTHBAR_FILL))
    {
        tomentdatapack.uiAssets[G_ASSET_HEALTHBAR_FILL]->texture = SDL_ConvertSurface(temp1, win_surface->format, 0);
        SDL_SetColorKey(tomentdatapack.uiAssets[G_ASSET_HEALTHBAR_FILL]->texture, SDL_TRUE, r_transparencyColor);    // Make transparency color for blitting
    }
    else
        printf("FATAL ERROR! Engine Default \"%d\" failed to load. Further behaviour is undefined.\n", IMG_ID_EDEFAULT_1);
    SDL_FreeSurface(temp1);

    // MANABAR_EMPTY
    offset = tomentdatapack.IMGArch.tocOffset + (tomentdatapack.IMGArch.toc[IMG_ID_MANABAR_EMPTY].startingOffset);
    sdlWops = SDL_RWFromConstMem((byte*)tomentdatapack.IMGArch.buffer+offset, tomentdatapack.IMGArch.toc[IMG_ID_MANABAR_EMPTY].size);
    temp1 = SDL_LoadBMP_RW(sdlWops, SDL_TRUE);
    if(D_CheckTextureLoaded(temp1, IMG_ID_MANABAR_EMPTY))
    {
        tomentdatapack.uiAssets[G_ASSET_MANABAR_EMPTY]->texture = SDL_ConvertSurface(temp1, win_surface->format, 0);
        SDL_SetColorKey(tomentdatapack.uiAssets[G_ASSET_MANABAR_EMPTY]->texture, SDL_TRUE, r_transparencyColor);    // Make transparency color for blitting
    }
    else
        printf("FATAL ERROR! Engine Default \"%d\" failed to load. Further behaviour is undefined.\n", IMG_ID_EDEFAULT_1);
    SDL_FreeSurface(temp1);

    // MANABAR_FILL
    offset = tomentdatapack.IMGArch.tocOffset + (tomentdatapack.IMGArch.toc[IMG_ID_MANABAR_FILL].startingOffset);
    sdlWops = SDL_RWFromConstMem((byte*)tomentdatapack.IMGArch.buffer+offset, tomentdatapack.IMGArch.toc[IMG_ID_MANABAR_FILL].size);
    temp1 = SDL_LoadBMP_RW(sdlWops, SDL_TRUE);
    if(D_CheckTextureLoaded(temp1, IMG_ID_MANABAR_FILL))
    {
        tomentdatapack.uiAssets[G_ASSET_MANABAR_FILL]->texture = SDL_ConvertSurface(temp1, win_surface->format, 0);
        SDL_SetColorKey(tomentdatapack.uiAssets[G_ASSET_MANABAR_FILL]->texture, SDL_TRUE, r_transparencyColor);    // Make transparency color for blitting
    }
    else
        printf("FATAL ERROR! Engine Default \"%d\" failed to load. Further behaviour is undefined.\n", IMG_ID_EDEFAULT_1);
    SDL_FreeSurface(temp1);

    // Load Icons
    // Icon Fist
    offset = tomentdatapack.IMGArch.tocOffset + (tomentdatapack.IMGArch.toc[IMG_ID_ICON_FISTS].startingOffset);
    sdlWops = SDL_RWFromConstMem((byte*)tomentdatapack.IMGArch.buffer+offset, tomentdatapack.IMGArch.toc[IMG_ID_ICON_FISTS].size);
    temp1 = SDL_LoadBMP_RW(sdlWops, SDL_TRUE);
    if(D_CheckTextureLoaded(temp1, IMG_ID_ICON_FISTS))
    {
        tomentdatapack.uiAssets[G_ASSET_ICON_FISTS]->texture = SDL_ConvertSurface(temp1, win_surface->format, 0);
        SDL_SetColorKey(tomentdatapack.uiAssets[G_ASSET_ICON_FISTS]->texture, SDL_TRUE, r_transparencyColor);    // Make transparency color for blitting
    }
    else
        printf("FATAL ERROR! Engine Default \"%d\" failed to load. Further behaviour is undefined.\n", IMG_ID_EDEFAULT_1);
    SDL_FreeSurface(temp1);

    // Icon Axe
    offset = tomentdatapack.IMGArch.tocOffset + (tomentdatapack.IMGArch.toc[IMG_ID_ICON_AXE].startingOffset);
    sdlWops = SDL_RWFromConstMem((byte*)tomentdatapack.IMGArch.buffer+offset, tomentdatapack.IMGArch.toc[IMG_ID_ICON_AXE].size);
    temp1 = SDL_LoadBMP_RW(sdlWops, SDL_TRUE);
    if(D_CheckTextureLoaded(temp1, IMG_ID_ICON_AXE))
    {
        tomentdatapack.uiAssets[G_ASSET_ICON_AXE]->texture = SDL_ConvertSurface(temp1, win_surface->format, 0);
        SDL_SetColorKey(tomentdatapack.uiAssets[G_ASSET_ICON_AXE]->texture, SDL_TRUE, r_transparencyColor);    // Make transparency color for blitting
    }
    else
        printf("FATAL ERROR! Engine Default \"%d\" failed to load. Further behaviour is undefined.\n", IMG_ID_EDEFAULT_1);
    SDL_FreeSurface(temp1);

    // Icon Spell Fireball
    offset = tomentdatapack.IMGArch.tocOffset + (tomentdatapack.IMGArch.toc[IMG_ID_ICON_SPELL_FIREBALL1].startingOffset);
    sdlWops = SDL_RWFromConstMem((byte*)tomentdatapack.IMGArch.buffer+offset, tomentdatapack.IMGArch.toc[IMG_ID_ICON_SPELL_FIREBALL1].size);
    temp1 = SDL_LoadBMP_RW(sdlWops, SDL_TRUE);
    if(D_CheckTextureLoaded(temp1, IMG_ID_ICON_SPELL_FIREBALL1))
    {
        tomentdatapack.uiAssets[G_ASSET_ICON_SPELL_FIREBALL1]->texture = SDL_ConvertSurface(temp1, win_surface->format, 0);
        SDL_SetColorKey(tomentdatapack.uiAssets[G_ASSET_ICON_SPELL_FIREBALL1]->texture, SDL_TRUE, r_transparencyColor);    // Make transparency color for blitting
    }
    else
        printf("FATAL ERROR! Engine Default \"%d\" failed to load. Further behaviour is undefined.\n", IMG_ID_EDEFAULT_1);
    SDL_FreeSurface(temp1);

    // Crosshair
    offset = tomentdatapack.IMGArch.tocOffset + (tomentdatapack.IMGArch.toc[IMG_ID_UI_CROSSHAIR].startingOffset);
    sdlWops = SDL_RWFromConstMem((byte*)tomentdatapack.IMGArch.buffer+offset, tomentdatapack.IMGArch.toc[IMG_ID_UI_CROSSHAIR].size);
    temp1 = SDL_LoadBMP_RW(sdlWops, SDL_TRUE);
    if(D_CheckTextureLoaded(temp1, IMG_ID_UI_CROSSHAIR))
    {
        tomentdatapack.uiAssets[G_ASSET_UI_CROSSHAIR]->texture = SDL_ConvertSurface(temp1, win_surface->format, 0);
        SDL_SetColorKey(tomentdatapack.uiAssets[G_ASSET_UI_CROSSHAIR]->texture, SDL_TRUE, r_transparencyColor);    // Make transparency color for blitting
    }
    else
        printf("FATAL ERROR! Engine Default \"%d\" failed to load. Further behaviour is undefined.\n", IMG_ID_EDEFAULT_1);
    SDL_FreeSurface(temp1);

    // Icon Spell Ice Dart
    offset = tomentdatapack.IMGArch.tocOffset + (tomentdatapack.IMGArch.toc[IMG_ID_ICON_SPELL_ICEDART1].startingOffset);
    sdlWops = SDL_RWFromConstMem((byte*)tomentdatapack.IMGArch.buffer+offset, tomentdatapack.IMGArch.toc[IMG_ID_ICON_SPELL_ICEDART1].size);
    temp1 = SDL_LoadBMP_RW(sdlWops, SDL_TRUE);
    if(D_CheckTextureLoaded(temp1, IMG_ID_ICON_SPELL_ICEDART1))
    {
        tomentdatapack.uiAssets[G_ASSET_ICON_SPELL_ICEDART1]->texture = SDL_ConvertSurface(temp1, win_surface->format, 0);
        SDL_SetColorKey(tomentdatapack.uiAssets[G_ASSET_ICON_SPELL_ICEDART1]->texture, SDL_TRUE, r_transparencyColor);    // Make transparency color for blitting
    }
    else
        printf("FATAL ERROR! Engine Default \"%d\" failed to load. Further behaviour is undefined.\n", IMG_ID_EDEFAULT_1);
    SDL_FreeSurface(temp1);

    // Crosshair Hit
    offset = tomentdatapack.IMGArch.tocOffset + (tomentdatapack.IMGArch.toc[IMG_ID_UI_CROSSHAIR_HIT].startingOffset);
    sdlWops = SDL_RWFromConstMem((byte*)tomentdatapack.IMGArch.buffer+offset, tomentdatapack.IMGArch.toc[IMG_ID_UI_CROSSHAIR_HIT].size);
    temp1 = SDL_LoadBMP_RW(sdlWops, SDL_TRUE);
    if(D_CheckTextureLoaded(temp1, IMG_ID_UI_CROSSHAIR_HIT))
    {
        tomentdatapack.uiAssets[G_ASSET_UI_CROSSHAIR_HIT]->texture = SDL_ConvertSurface(temp1, win_surface->format, 0);
        SDL_SetColorKey(tomentdatapack.uiAssets[G_ASSET_UI_CROSSHAIR_HIT]->texture, SDL_TRUE, r_transparencyColor);    // Make transparency color for blitting
    }
    else
        printf("FATAL ERROR! Engine Default \"%d\" failed to load. Further behaviour is undefined.\n", IMG_ID_EDEFAULT_1);
    SDL_FreeSurface(temp1);

    // BOSS_HEALTHBAR_EMPTY
    offset = tomentdatapack.IMGArch.tocOffset + (tomentdatapack.IMGArch.toc[IMG_ID_BOSS_HEALTHBAR_EMPTY].startingOffset);
    sdlWops = SDL_RWFromConstMem((byte*)tomentdatapack.IMGArch.buffer+offset, tomentdatapack.IMGArch.toc[IMG_ID_BOSS_HEALTHBAR_EMPTY].size);
    temp1 = SDL_LoadBMP_RW(sdlWops, SDL_TRUE);
    if(D_CheckTextureLoaded(temp1, IMG_ID_BOSS_HEALTHBAR_EMPTY))
        tomentdatapack.uiAssets[G_ASSET_BOSS_HEALTHBAR_EMPTY]->texture = SDL_ConvertSurface(temp1, win_surface->format, 0);
    else
        printf("FATAL ERROR! Engine Default \"%d\" failed to load. Further behaviour is undefined.\n", IMG_ID_EDEFAULT_1);
    SDL_SetColorKey(tomentdatapack.uiAssets[G_ASSET_BOSS_HEALTHBAR_EMPTY]->texture, SDL_TRUE, r_transparencyColor);    // Make transparency color for blitting
    SDL_FreeSurface(temp1);

    // BOSS_HEALTHBAR_FILL
    offset = tomentdatapack.IMGArch.tocOffset + (tomentdatapack.IMGArch.toc[IMG_ID_BOSS_HEALTHBAR_FILL].startingOffset);
    sdlWops = SDL_RWFromConstMem((byte*)tomentdatapack.IMGArch.buffer+offset, tomentdatapack.IMGArch.toc[IMG_ID_BOSS_HEALTHBAR_FILL].size);
    temp1 = SDL_LoadBMP_RW(sdlWops, SDL_TRUE);
    if(D_CheckTextureLoaded(temp1, IMG_ID_BOSS_HEALTHBAR_FILL))
        tomentdatapack.uiAssets[G_ASSET_BOSS_HEALTHBAR_FILL]->texture = SDL_ConvertSurface(temp1, win_surface->format, 0);
    else
        printf("FATAL ERROR! Engine Default \"%d\" failed to load. Further behaviour is undefined.\n", IMG_ID_EDEFAULT_1);
    SDL_SetColorKey(tomentdatapack.uiAssets[G_ASSET_BOSS_HEALTHBAR_FILL]->texture, SDL_TRUE, r_transparencyColor);    // Make transparency color for blitting
    SDL_FreeSurface(temp1);

    // Icon Greatsword
    offset = tomentdatapack.IMGArch.tocOffset + (tomentdatapack.IMGArch.toc[IMG_ID_ICON_GREATSWORD].startingOffset);
    sdlWops = SDL_RWFromConstMem((byte*)tomentdatapack.IMGArch.buffer+offset, tomentdatapack.IMGArch.toc[IMG_ID_ICON_GREATSWORD].size);
    temp1 = SDL_LoadBMP_RW(sdlWops, SDL_TRUE);
    if(D_CheckTextureLoaded(temp1, IMG_ID_ICON_GREATSWORD))
    {
        tomentdatapack.uiAssets[G_ASSET_ICON_GREATSWORD]->texture = SDL_ConvertSurface(temp1, win_surface->format, 0);
        SDL_SetColorKey(tomentdatapack.uiAssets[G_ASSET_ICON_GREATSWORD]->texture, SDL_TRUE, r_transparencyColor);    // Make transparency color for blitting
    }
    else
        printf("FATAL ERROR! Engine Default \"%d\" failed to load. Further behaviour is undefined.\n", IMG_ID_EDEFAULT_1);
    SDL_FreeSurface(temp1);

    // Tank class potrait
    offset = tomentdatapack.IMGArch.tocOffset + (tomentdatapack.IMGArch.toc[IMG_ID_TANK_CLASS_POTRAIT].startingOffset);
    sdlWops = SDL_RWFromConstMem((byte*)tomentdatapack.IMGArch.buffer+offset, tomentdatapack.IMGArch.toc[IMG_ID_TANK_CLASS_POTRAIT].size);
    temp1 = SDL_LoadBMP_RW(sdlWops, SDL_TRUE);
    if(D_CheckTextureLoaded(temp1, IMG_ID_TANK_CLASS_POTRAIT))
    {
        tomentdatapack.uiAssets[G_ASSET_CLASS_POTRAIT_TANK]->texture = SDL_ConvertSurface(temp1, win_surface->format, 0);
        SDL_SetColorKey(tomentdatapack.uiAssets[G_ASSET_CLASS_POTRAIT_TANK]->texture, SDL_TRUE, r_transparencyColor);    // Make transparency color for blitting
    }
    else
        printf("FATAL ERROR! Engine Default \"%d\" failed to load. Further behaviour is undefined.\n", IMG_ID_EDEFAULT_1);
    SDL_FreeSurface(temp1);

     // Healer class potrait
    offset = tomentdatapack.IMGArch.tocOffset + (tomentdatapack.IMGArch.toc[IMG_ID_HEALER_CLASS_POTRAIT].startingOffset);
    sdlWops = SDL_RWFromConstMem((byte*)tomentdatapack.IMGArch.buffer+offset, tomentdatapack.IMGArch.toc[IMG_ID_HEALER_CLASS_POTRAIT].size);
    temp1 = SDL_LoadBMP_RW(sdlWops, SDL_TRUE);
    if(D_CheckTextureLoaded(temp1, IMG_ID_HEALER_CLASS_POTRAIT))
    {
        tomentdatapack.uiAssets[G_ASSET_CLASS_POTRAIT_HEALER]->texture = SDL_ConvertSurface(temp1, win_surface->format, 0);
        SDL_SetColorKey(tomentdatapack.uiAssets[G_ASSET_CLASS_POTRAIT_HEALER]->texture, SDL_TRUE, r_transparencyColor);    // Make transparency color for blitting
    }
    else
        printf("FATAL ERROR! Engine Default \"%d\" failed to load. Further behaviour is undefined.\n", IMG_ID_EDEFAULT_1);
    SDL_FreeSurface(temp1);

     // DPS class potrait
    offset = tomentdatapack.IMGArch.tocOffset + (tomentdatapack.IMGArch.toc[IMG_ID_DPS_CLASS_POTRAIT].startingOffset);
    sdlWops = SDL_RWFromConstMem((byte*)tomentdatapack.IMGArch.buffer+offset, tomentdatapack.IMGArch.toc[IMG_ID_DPS_CLASS_POTRAIT].size);
    temp1 = SDL_LoadBMP_RW(sdlWops, SDL_TRUE);
    if(D_CheckTextureLoaded(temp1, IMG_ID_DPS_CLASS_POTRAIT))
    {
        tomentdatapack.uiAssets[G_ASSET_CLASS_POTRAIT_DPS]->texture = SDL_ConvertSurface(temp1, win_surface->format, 0);
        SDL_SetColorKey(tomentdatapack.uiAssets[G_ASSET_CLASS_POTRAIT_DPS]->texture, SDL_TRUE, r_transparencyColor);    // Make transparency color for blitting
    }
    else
        printf("FATAL ERROR! Engine Default \"%d\" failed to load. Further behaviour is undefined.\n", IMG_ID_EDEFAULT_1);
    SDL_FreeSurface(temp1);


    // Tank class icon
    offset = tomentdatapack.IMGArch.tocOffset + (tomentdatapack.IMGArch.toc[IMG_ID_TANK_CLASS_ICON].startingOffset);
    sdlWops = SDL_RWFromConstMem((byte*)tomentdatapack.IMGArch.buffer+offset, tomentdatapack.IMGArch.toc[IMG_ID_TANK_CLASS_ICON].size);
    temp1 = SDL_LoadBMP_RW(sdlWops, SDL_TRUE);
    if(D_CheckTextureLoaded(temp1, IMG_ID_TANK_CLASS_ICON))
    {
        tomentdatapack.uiAssets[G_ASSET_CLASS_ICON_TANK]->texture = SDL_ConvertSurface(temp1, win_surface->format, 0);
        SDL_SetColorKey(tomentdatapack.uiAssets[G_ASSET_CLASS_ICON_TANK]->texture, SDL_TRUE, r_transparencyColor);    // Make transparency color for blitting
    }
    else
        printf("FATAL ERROR! Engine Default \"%d\" failed to load. Further behaviour is undefined.\n", IMG_ID_EDEFAULT_1);
    SDL_FreeSurface(temp1);

    // Healer class icon
    offset = tomentdatapack.IMGArch.tocOffset + (tomentdatapack.IMGArch.toc[IMG_ID_HEALER_CLASS_ICON].startingOffset);
    sdlWops = SDL_RWFromConstMem((byte*)tomentdatapack.IMGArch.buffer+offset, tomentdatapack.IMGArch.toc[IMG_ID_HEALER_CLASS_ICON].size);
    temp1 = SDL_LoadBMP_RW(sdlWops, SDL_TRUE);
    if(D_CheckTextureLoaded(temp1, IMG_ID_HEALER_CLASS_ICON))
    {
        tomentdatapack.uiAssets[G_ASSET_CLASS_ICON_HEALER]->texture = SDL_ConvertSurface(temp1, win_surface->format, 0);
        SDL_SetColorKey(tomentdatapack.uiAssets[G_ASSET_CLASS_ICON_HEALER]->texture, SDL_TRUE, r_transparencyColor);    // Make transparency color for blitting
    }
    else
        printf("FATAL ERROR! Engine Default \"%d\" failed to load. Further behaviour is undefined.\n", IMG_ID_EDEFAULT_1);
    SDL_FreeSurface(temp1);

    // DPS class icon
    offset = tomentdatapack.IMGArch.tocOffset + (tomentdatapack.IMGArch.toc[IMG_ID_DPS_CLASS_ICON].startingOffset);
    sdlWops = SDL_RWFromConstMem((byte*)tomentdatapack.IMGArch.buffer+offset, tomentdatapack.IMGArch.toc[IMG_ID_DPS_CLASS_ICON].size);
    temp1 = SDL_LoadBMP_RW(sdlWops, SDL_TRUE);
    if(D_CheckTextureLoaded(temp1, IMG_ID_DPS_CLASS_ICON))
    {
        tomentdatapack.uiAssets[G_ASSET_CLASS_ICON_DPS]->texture = SDL_ConvertSurface(temp1, win_surface->format, 0);
        SDL_SetColorKey(tomentdatapack.uiAssets[G_ASSET_CLASS_ICON_DPS]->texture, SDL_TRUE, r_transparencyColor);    // Make transparency color for blitting
    }
    else
        printf("FATAL ERROR! Engine Default \"%d\" failed to load. Further behaviour is undefined.\n", IMG_ID_EDEFAULT_1);
    SDL_FreeSurface(temp1);

    // Class icon selection
    offset = tomentdatapack.IMGArch.tocOffset + (tomentdatapack.IMGArch.toc[IMG_ID_CLASS_ICON_SELECTION].startingOffset);
    sdlWops = SDL_RWFromConstMem((byte*)tomentdatapack.IMGArch.buffer+offset, tomentdatapack.IMGArch.toc[IMG_ID_CLASS_ICON_SELECTION].size);
    temp1 = SDL_LoadBMP_RW(sdlWops, SDL_TRUE);
    if(D_CheckTextureLoaded(temp1, IMG_ID_CLASS_ICON_SELECTION))
    {
        tomentdatapack.uiAssets[G_ASSET_CLASS_ICON_SELECTION]->texture = SDL_ConvertSurface(temp1, win_surface->format, 0);
        SDL_SetColorKey(tomentdatapack.uiAssets[G_ASSET_CLASS_ICON_SELECTION]->texture, SDL_TRUE, r_transparencyColor);    // Make transparency color for blitting
    }
    else
        printf("FATAL ERROR! Engine Default \"%d\" failed to load. Further behaviour is undefined.\n", IMG_ID_EDEFAULT_1);
    SDL_FreeSurface(temp1);

    // Class icon selection disabled
    offset = tomentdatapack.IMGArch.tocOffset + (tomentdatapack.IMGArch.toc[IMG_ID_CLASS_ICON_SELECTION_DISABLED].startingOffset);
    sdlWops = SDL_RWFromConstMem((byte*)tomentdatapack.IMGArch.buffer+offset, tomentdatapack.IMGArch.toc[IMG_ID_CLASS_ICON_SELECTION_DISABLED].size);
    temp1 = SDL_LoadBMP_RW(sdlWops, SDL_TRUE);
    if(D_CheckTextureLoaded(temp1, IMG_ID_CLASS_ICON_SELECTION_DISABLED))
    {
        tomentdatapack.uiAssets[G_ASSET_CLASS_ICON_SELECTION_DISABLED]->texture = SDL_ConvertSurface(temp1, win_surface->format, 0);
        SDL_SetColorKey(tomentdatapack.uiAssets[G_ASSET_CLASS_ICON_SELECTION_DISABLED]->texture, SDL_TRUE, r_transparencyColor);    // Make transparency color for blitting
    }
    else
        printf("FATAL ERROR! Engine Default \"%d\" failed to load. Further behaviour is undefined.\n", IMG_ID_EDEFAULT_1);
    SDL_FreeSurface(temp1);

    // Ready Icon
    offset = tomentdatapack.IMGArch.tocOffset + (tomentdatapack.IMGArch.toc[IMG_ID_ICON_READY].startingOffset);
    sdlWops = SDL_RWFromConstMem((byte*)tomentdatapack.IMGArch.buffer+offset, tomentdatapack.IMGArch.toc[IMG_ID_ICON_READY].size);
    temp1 = SDL_LoadBMP_RW(sdlWops, SDL_TRUE);
    if(D_CheckTextureLoaded(temp1, IMG_ID_ICON_READY))
    {
        tomentdatapack.uiAssets[G_ASSET_ICON_READY]->texture = SDL_ConvertSurface(temp1, win_surface->format, 0);
        SDL_SetColorKey(tomentdatapack.uiAssets[G_ASSET_ICON_READY]->texture, SDL_TRUE, r_transparencyColor);    // Make transparency color for blitting
    }
    else
        printf("FATAL ERROR! Engine Default \"%d\" failed to load. Further behaviour is undefined.\n", IMG_ID_EDEFAULT_1);
    SDL_FreeSurface(temp1);

    // Not Ready Icon
    offset = tomentdatapack.IMGArch.tocOffset + (tomentdatapack.IMGArch.toc[IMG_ID_ICON_NOTREADY].startingOffset);
    sdlWops = SDL_RWFromConstMem((byte*)tomentdatapack.IMGArch.buffer+offset, tomentdatapack.IMGArch.toc[IMG_ID_ICON_NOTREADY].size);
    temp1 = SDL_LoadBMP_RW(sdlWops, SDL_TRUE);
    if(D_CheckTextureLoaded(temp1, IMG_ID_ICON_NOTREADY))
    {
        tomentdatapack.uiAssets[G_ASSET_ICON_NOTREADY]->texture = SDL_ConvertSurface(temp1, win_surface->format, 0);
        SDL_SetColorKey(tomentdatapack.uiAssets[G_ASSET_ICON_NOTREADY]->texture, SDL_TRUE, r_transparencyColor);    // Make transparency color for blitting
    }
    else
        printf("FATAL ERROR! Engine Default \"%d\" failed to load. Further behaviour is undefined.\n", IMG_ID_EDEFAULT_1);
    SDL_FreeSurface(temp1);

    // Icon Mace
    offset = tomentdatapack.IMGArch.tocOffset + (tomentdatapack.IMGArch.toc[IMG_ID_ICON_MACE].startingOffset);
    sdlWops = SDL_RWFromConstMem((byte*)tomentdatapack.IMGArch.buffer+offset, tomentdatapack.IMGArch.toc[IMG_ID_ICON_MACE].size);
    temp1 = SDL_LoadBMP_RW(sdlWops, SDL_TRUE);
    if(D_CheckTextureLoaded(temp1, IMG_ID_ICON_MACE))
    {
        tomentdatapack.uiAssets[G_ASSET_ICON_MACE]->texture = SDL_ConvertSurface(temp1, win_surface->format, 0);
        SDL_SetColorKey(tomentdatapack.uiAssets[G_ASSET_ICON_MACE]->texture, SDL_TRUE, r_transparencyColor);    // Make transparency color for blitting
    }
    else
        printf("FATAL ERROR! Engine Default \"%d\" failed to load. Further behaviour is undefined.\n", IMG_ID_EDEFAULT_1);
    SDL_FreeSurface(temp1);

    // IMG_ID_SKILL_ICON_TANK_SHIELD_SLAM
    offset = tomentdatapack.IMGArch.tocOffset + (tomentdatapack.IMGArch.toc[IMG_ID_SKILL_ICON_TANK_SHIELD_SLAM].startingOffset);
    sdlWops = SDL_RWFromConstMem((byte*)tomentdatapack.IMGArch.buffer+offset, tomentdatapack.IMGArch.toc[IMG_ID_SKILL_ICON_TANK_SHIELD_SLAM].size);
    temp1 = SDL_LoadBMP_RW(sdlWops, SDL_TRUE);
    if(D_CheckTextureLoaded(temp1, IMG_ID_SKILL_ICON_TANK_SHIELD_SLAM))
    {
        tomentdatapack.uiAssets[G_ASSET_SKILL_ICON_TANK_SHIELD_SLAM]->texture = SDL_ConvertSurface(temp1, win_surface->format, 0);
        SDL_SetColorKey(tomentdatapack.uiAssets[G_ASSET_SKILL_ICON_TANK_SHIELD_SLAM]->texture, SDL_TRUE, r_transparencyColor);    // Make transparency color for blitting
    }
    else
        printf("FATAL ERROR! Engine Default \"%d\" failed to load. Further behaviour is undefined.\n", IMG_ID_EDEFAULT_1);
    SDL_FreeSurface(temp1);

    // IMG_ID_SKILL_ICON_TANK_SHIELD_BLOCK
    offset = tomentdatapack.IMGArch.tocOffset + (tomentdatapack.IMGArch.toc[IMG_ID_SKILL_ICON_TANK_SHIELD_BLOCK].startingOffset);
    sdlWops = SDL_RWFromConstMem((byte*)tomentdatapack.IMGArch.buffer+offset, tomentdatapack.IMGArch.toc[IMG_ID_SKILL_ICON_TANK_SHIELD_BLOCK].size);
    temp1 = SDL_LoadBMP_RW(sdlWops, SDL_TRUE);
    if(D_CheckTextureLoaded(temp1, IMG_ID_SKILL_ICON_TANK_SHIELD_BLOCK))
    {
        tomentdatapack.uiAssets[G_ASSET_SKILL_ICON_TANK_SHIELD_BLOCK]->texture = SDL_ConvertSurface(temp1, win_surface->format, 0);
        SDL_SetColorKey(tomentdatapack.uiAssets[G_ASSET_SKILL_ICON_TANK_SHIELD_BLOCK]->texture, SDL_TRUE, r_transparencyColor);    // Make transparency color for blitting
    }
    else
        printf("FATAL ERROR! Engine Default \"%d\" failed to load. Further behaviour is undefined.\n", IMG_ID_EDEFAULT_1);
    SDL_FreeSurface(temp1);

    // IMG_ID_SKILL_ICON_TANK_CONSACRATION
    offset = tomentdatapack.IMGArch.tocOffset + (tomentdatapack.IMGArch.toc[IMG_ID_SKILL_ICON_TANK_CONSACRATION].startingOffset);
    sdlWops = SDL_RWFromConstMem((byte*)tomentdatapack.IMGArch.buffer+offset, tomentdatapack.IMGArch.toc[IMG_ID_SKILL_ICON_TANK_CONSACRATION].size);
    temp1 = SDL_LoadBMP_RW(sdlWops, SDL_TRUE);
    if(D_CheckTextureLoaded(temp1, IMG_ID_SKILL_ICON_TANK_CONSACRATION))
    {
        tomentdatapack.uiAssets[G_ASSET_SKILL_ICON_TANK_CONSACRATION]->texture = SDL_ConvertSurface(temp1, win_surface->format, 0);
        SDL_SetColorKey(tomentdatapack.uiAssets[G_ASSET_SKILL_ICON_TANK_CONSACRATION]->texture, SDL_TRUE, r_transparencyColor);    // Make transparency color for blitting
    }
    else
        printf("FATAL ERROR! Engine Default \"%d\" failed to load. Further behaviour is undefined.\n", IMG_ID_EDEFAULT_1);
    SDL_FreeSurface(temp1);

    // Skill Icon Empty
    offset = tomentdatapack.IMGArch.tocOffset + (tomentdatapack.IMGArch.toc[IMG_ID_SKILL_ICON_EMPTY].startingOffset);
    sdlWops = SDL_RWFromConstMem((byte*)tomentdatapack.IMGArch.buffer+offset, tomentdatapack.IMGArch.toc[IMG_ID_SKILL_ICON_EMPTY].size);
    temp1 = SDL_LoadBMP_RW(sdlWops, SDL_TRUE);
    if(D_CheckTextureLoaded(temp1, IMG_ID_SKILL_ICON_EMPTY))
    {
        tomentdatapack.uiAssets[G_ASSET_SKILL_ICON_EMPTY]->texture = SDL_ConvertSurface(temp1, win_surface->format, 0);
        SDL_SetColorKey(tomentdatapack.uiAssets[G_ASSET_SKILL_ICON_EMPTY]->texture, SDL_TRUE, r_transparencyColor);    // Make transparency color for blitting
    }
    else
        printf("FATAL ERROR! Engine Default \"%d\" failed to load. Further behaviour is undefined.\n", IMG_ID_EDEFAULT_1);
    SDL_FreeSurface(temp1);

    // Self heal icon
    offset = tomentdatapack.IMGArch.tocOffset + (tomentdatapack.IMGArch.toc[IMG_ID_SKILL_ICON_HEALER_SELF_HEAL].startingOffset);
    sdlWops = SDL_RWFromConstMem((byte*)tomentdatapack.IMGArch.buffer+offset, tomentdatapack.IMGArch.toc[IMG_ID_SKILL_ICON_HEALER_SELF_HEAL].size);
    temp1 = SDL_LoadBMP_RW(sdlWops, SDL_TRUE);
    if(D_CheckTextureLoaded(temp1, IMG_ID_SKILL_ICON_HEALER_SELF_HEAL))
    {
        tomentdatapack.uiAssets[G_ASSET_SKILL_ICON_HEALER_SELF_HEAL]->texture = SDL_ConvertSurface(temp1, win_surface->format, 0);
        SDL_SetColorKey(tomentdatapack.uiAssets[G_ASSET_SKILL_ICON_HEALER_SELF_HEAL]->texture, SDL_TRUE, r_transparencyColor);    // Make transparency color for blitting
    }
    else
        printf("FATAL ERROR! Engine Default \"%d\" failed to load. Further behaviour is undefined.\n", IMG_ID_EDEFAULT_1);
    SDL_FreeSurface(temp1);

    // Concentrated heal icon
    offset = tomentdatapack.IMGArch.tocOffset + (tomentdatapack.IMGArch.toc[IMG_ID_SKILL_ICON_HEALER_CONCENTRATED_HEAL].startingOffset);
    sdlWops = SDL_RWFromConstMem((byte*)tomentdatapack.IMGArch.buffer+offset, tomentdatapack.IMGArch.toc[IMG_ID_SKILL_ICON_HEALER_CONCENTRATED_HEAL].size);
    temp1 = SDL_LoadBMP_RW(sdlWops, SDL_TRUE);
    if(D_CheckTextureLoaded(temp1, IMG_ID_SKILL_ICON_HEALER_CONCENTRATED_HEAL))
    {
        tomentdatapack.uiAssets[G_ASSET_SKILL_ICON_HEALER_CONCENTRATED_HEAL]->texture = SDL_ConvertSurface(temp1, win_surface->format, 0);
        SDL_SetColorKey(tomentdatapack.uiAssets[G_ASSET_SKILL_ICON_HEALER_CONCENTRATED_HEAL]->texture, SDL_TRUE, r_transparencyColor);    // Make transparency color for blitting
    }
    else
        printf("FATAL ERROR! Engine Default \"%d\" failed to load. Further behaviour is undefined.\n", IMG_ID_EDEFAULT_1);
    SDL_FreeSurface(temp1);

    // Divine intervention icon
    offset = tomentdatapack.IMGArch.tocOffset + (tomentdatapack.IMGArch.toc[IMG_ID_SKILL_ICON_HEALER_DIVINE_INTERVENTION].startingOffset);
    sdlWops = SDL_RWFromConstMem((byte*)tomentdatapack.IMGArch.buffer+offset, tomentdatapack.IMGArch.toc[IMG_ID_SKILL_ICON_HEALER_DIVINE_INTERVENTION].size);
    temp1 = SDL_LoadBMP_RW(sdlWops, SDL_TRUE);
    if(D_CheckTextureLoaded(temp1, IMG_ID_SKILL_ICON_HEALER_DIVINE_INTERVENTION))
    {
        tomentdatapack.uiAssets[G_ASSET_SKILL_ICON_HEALER_DIVINE_INTERVENTION]->texture = SDL_ConvertSurface(temp1, win_surface->format, 0);
        SDL_SetColorKey(tomentdatapack.uiAssets[G_ASSET_SKILL_ICON_HEALER_DIVINE_INTERVENTION]->texture, SDL_TRUE, r_transparencyColor);    // Make transparency color for blitting
    }
    else
        printf("FATAL ERROR! Engine Default \"%d\" failed to load. Further behaviour is undefined.\n", IMG_ID_EDEFAULT_1);
    SDL_FreeSurface(temp1);

    // Cheapshot Icon
    offset = tomentdatapack.IMGArch.tocOffset + (tomentdatapack.IMGArch.toc[IMG_ID_SKILL_ICON_DPS_CHEAPSHOT].startingOffset);
    sdlWops = SDL_RWFromConstMem((byte*)tomentdatapack.IMGArch.buffer+offset, tomentdatapack.IMGArch.toc[IMG_ID_SKILL_ICON_DPS_CHEAPSHOT].size);
    temp1 = SDL_LoadBMP_RW(sdlWops, SDL_TRUE);
    if(D_CheckTextureLoaded(temp1, IMG_ID_SKILL_ICON_DPS_CHEAPSHOT))
    {
        tomentdatapack.uiAssets[G_ASSET_SKILL_ICON_DPS_CHEAPSHOT]->texture = SDL_ConvertSurface(temp1, win_surface->format, 0);
        SDL_SetColorKey(tomentdatapack.uiAssets[G_ASSET_SKILL_ICON_DPS_CHEAPSHOT]->texture, SDL_TRUE, r_transparencyColor);    // Make transparency color for blitting
    }
    else
        printf("FATAL ERROR! Engine Default \"%d\" failed to load. Further behaviour is undefined.\n", IMG_ID_EDEFAULT_1);
    SDL_FreeSurface(temp1);

    // Obliterate Icon
    offset = tomentdatapack.IMGArch.tocOffset + (tomentdatapack.IMGArch.toc[IMG_ID_SKILL_ICON_DPS_OBLITERATE].startingOffset);
    sdlWops = SDL_RWFromConstMem((byte*)tomentdatapack.IMGArch.buffer+offset, tomentdatapack.IMGArch.toc[IMG_ID_SKILL_ICON_DPS_OBLITERATE].size);
    temp1 = SDL_LoadBMP_RW(sdlWops, SDL_TRUE);
    if(D_CheckTextureLoaded(temp1, IMG_ID_SKILL_ICON_DPS_OBLITERATE))
    {
        tomentdatapack.uiAssets[G_ASSET_SKILL_ICON_DPS_OBLITERATE]->texture = SDL_ConvertSurface(temp1, win_surface->format, 0);
        SDL_SetColorKey(tomentdatapack.uiAssets[G_ASSET_SKILL_ICON_DPS_OBLITERATE]->texture, SDL_TRUE, r_transparencyColor);    // Make transparency color for blitting
    }
    else
        printf("FATAL ERROR! Engine Default \"%d\" failed to load. Further behaviour is undefined.\n", IMG_ID_EDEFAULT_1);
    SDL_FreeSurface(temp1);

    // Split Icon
    offset = tomentdatapack.IMGArch.tocOffset + (tomentdatapack.IMGArch.toc[IMG_ID_SKILL_ICON_DPS_SPLIT].startingOffset);
    sdlWops = SDL_RWFromConstMem((byte*)tomentdatapack.IMGArch.buffer+offset, tomentdatapack.IMGArch.toc[IMG_ID_SKILL_ICON_DPS_SPLIT].size);
    temp1 = SDL_LoadBMP_RW(sdlWops, SDL_TRUE);
    if(D_CheckTextureLoaded(temp1, IMG_ID_SKILL_ICON_DPS_SPLIT))
    {
        tomentdatapack.uiAssets[G_ASSET_SKILL_ICON_DPS_SPLIT]->texture = SDL_ConvertSurface(temp1, win_surface->format, 0);
        SDL_SetColorKey(tomentdatapack.uiAssets[G_ASSET_SKILL_ICON_DPS_SPLIT]->texture, SDL_TRUE, r_transparencyColor);    // Make transparency color for blitting
    }
    else
        printf("FATAL ERROR! Engine Default \"%d\" failed to load. Further behaviour is undefined.\n", IMG_ID_EDEFAULT_1);
    SDL_FreeSurface(temp1);

    offset = tomentdatapack.IMGArch.tocOffset + (tomentdatapack.IMGArch.toc[IMG_ID_INPUTFIELD_01].startingOffset);
    sdlWops = SDL_RWFromConstMem((byte*)tomentdatapack.IMGArch.buffer+offset, tomentdatapack.IMGArch.toc[IMG_ID_INPUTFIELD_01].size);
    temp1 = SDL_LoadBMP_RW(sdlWops, SDL_TRUE);
    if(D_CheckTextureLoaded(temp1, IMG_ID_INPUTFIELD_01))
        tomentdatapack.uiAssets[M_ASSET_INPUTFIELD_01]->texture = SDL_ConvertSurface(temp1, win_surface->format, 0);
    else
        printf("FATAL ERROR! Engine Default \"%d\" failed to load. Further behaviour is undefined.\n", IMG_ID_EDEFAULT_1);
    SDL_SetColorKey(tomentdatapack.uiAssets[M_ASSET_INPUTFIELD_01]->texture, SDL_TRUE, r_transparencyColor);    // Make transparency color for blitting
    SDL_FreeSurface(temp1);

    offset = tomentdatapack.IMGArch.tocOffset + (tomentdatapack.IMGArch.toc[IMG_ID_INPUTFIELD_01_ACTIVE].startingOffset);
    sdlWops = SDL_RWFromConstMem((byte*)tomentdatapack.IMGArch.buffer+offset, tomentdatapack.IMGArch.toc[IMG_ID_INPUTFIELD_01_ACTIVE].size);
    temp1 = SDL_LoadBMP_RW(sdlWops, SDL_TRUE);
    if(D_CheckTextureLoaded(temp1, IMG_ID_INPUTFIELD_01_ACTIVE))
        tomentdatapack.uiAssets[M_ASSET_INPUTFIELD_01_ACTIVE]->texture = SDL_ConvertSurface(temp1, win_surface->format, 0);
    else
        printf("FATAL ERROR! Engine Default \"%d\" failed to load. Further behaviour is undefined.\n", IMG_ID_EDEFAULT_1);
    SDL_SetColorKey(tomentdatapack.uiAssets[M_ASSET_INPUTFIELD_01_ACTIVE]->texture, SDL_TRUE, r_transparencyColor);    // Make transparency color for blitting
    SDL_FreeSurface(temp1);
}


void D_InitEnginesDefaults(void)
{    
    // Create Objects
    object_t* texturefallback = (object_t*)malloc(sizeof(object_t));
    tomentdatapack.enginesDefaultsLength = 1; // Set length

    D_InitObject(texturefallback);

    // Put objects in the datapack
    tomentdatapack.enginesDefaults[EDEFAULT_1] = texturefallback;

    // Fill objects
    // Convert all the surfaces that we will load in the same format as the win_surface
    SDL_Surface *temp1;     // Surface
    SDL_RWops* sdlWops;     // Structure to read bytes
    int offset;             // Offset in the img.archt

    // E_1
    offset = tomentdatapack.IMGArch.tocOffset + (tomentdatapack.IMGArch.toc[IMG_ID_EDEFAULT_1].startingOffset);
    sdlWops = SDL_RWFromConstMem((byte*)tomentdatapack.IMGArch.buffer+offset, tomentdatapack.IMGArch.toc[IMG_ID_EDEFAULT_1].size);
    temp1 = SDL_LoadBMP_RW(sdlWops, SDL_TRUE);
    if(D_CheckTextureLoaded(temp1, IMG_ID_EDEFAULT_1))
        tomentdatapack.enginesDefaults[EDEFAULT_1]->texture = SDL_ConvertSurface(temp1, win_surface->format, 0);
    else
        printf("FATAL ERROR! Engine Default \"%d\" failed to load. Further behaviour is undefined.\n", IMG_ID_EDEFAULT_1);
        
    SDL_FreeSurface(temp1);

    // Final sets
    D_SetObject(texturefallback, EDEFAULT_1, tomentdatapack.enginesDefaults[EDEFAULT_1]->texture, NULL);
}

void D_InitLoadTextures(void)
{
    // Create Objects
    textureObject_t* wallBrick1 = (textureObject_t*)malloc(sizeof(textureObject_t));
    textureObject_t* wallBrick1Dark = (textureObject_t*)malloc(sizeof(textureObject_t));
    textureObject_t* floorBrick1 = (textureObject_t*)malloc(sizeof(textureObject_t));
    textureObject_t* ceilingWood1 = (textureObject_t*)malloc(sizeof(textureObject_t));
    textureObject_t* wall2 = (textureObject_t*)malloc(sizeof(textureObject_t));
    textureObject_t* gate1 = (textureObject_t*)malloc(sizeof(textureObject_t));
    textureObject_t* gate1Alt = (textureObject_t*)malloc(sizeof(textureObject_t));
    textureObject_t* castleDoor = (textureObject_t*)malloc(sizeof(textureObject_t));
    textureObject_t* wall1Ladder = (textureObject_t*)malloc(sizeof(textureObject_t));
    textureObject_t* floorBrick2 = (textureObject_t*)malloc(sizeof(textureObject_t));
    textureObject_t* floorDirt1 = (textureObject_t*)malloc(sizeof(textureObject_t));
    textureObject_t* ice = (textureObject_t*)malloc(sizeof(textureObject_t));
    textureObject_t* iceConsacrated = (textureObject_t*)malloc(sizeof(textureObject_t));
    textureObject_t* violetVoid = (textureObject_t*)malloc(sizeof(textureObject_t));
    textureObject_t* icyGround = (textureObject_t*)malloc(sizeof(textureObject_t));

    tomentdatapack.texturesLength = 15; // Set length

    D_InitTextureAsset(wallBrick1);
    D_InitTextureAsset(wallBrick1Dark);
    D_InitTextureAsset(floorBrick1);
    D_InitTextureAsset(ceilingWood1);
    D_InitTextureAsset(wall2);
    D_InitTextureAsset(gate1);
    D_InitTextureAsset(gate1Alt);
    D_InitTextureAsset(castleDoor);
    D_InitTextureAsset(wall1Ladder);
    D_InitTextureAsset(floorBrick2);
    D_InitTextureAsset(floorDirt1);
    D_InitTextureAsset(ice);
    D_InitTextureAsset(iceConsacrated);
    D_InitTextureAsset(violetVoid);
    D_InitTextureAsset(icyGround);

    // Put objects in the datapack
    tomentdatapack.textures[TEXTURE_WallBrick1] = wallBrick1;
    tomentdatapack.textures[TEXTURE_WallBrick1Dark] = wallBrick1Dark;
    tomentdatapack.textures[TEXTURE_FloorBrick1] = floorBrick1;
    tomentdatapack.textures[TEXTURE_CeilingWood1] = ceilingWood1;
    tomentdatapack.textures[TEXTURE_Wall2] = wall2;
    tomentdatapack.textures[TEXTURE_Gate1] = gate1;
    tomentdatapack.textures[TEXTURE_Gate1Alt] = gate1Alt;
    tomentdatapack.textures[TEXTURE_CastleDoor] = castleDoor;
    tomentdatapack.textures[TEXTURE_Wall1Ladder] = wall1Ladder;
    tomentdatapack.textures[TEXTURE_FloorBrick2] = floorBrick2;
    tomentdatapack.textures[TEXTURE_FloorDirt1] = floorDirt1;
    tomentdatapack.textures[TEXTURE_Ice] = ice;
    tomentdatapack.textures[TEXTURE_IceConsacrated] = iceConsacrated;
    tomentdatapack.textures[TEXTURE_VioletVoid] = violetVoid;
    tomentdatapack.textures[TEXTURE_IcyGround] = icyGround;

    // Fill objects
    // Convert all the surfaces that we will load in the same format as the win_surface
    SDL_Surface *temp1;     // Surface
    SDL_RWops* sdlWops;     // Structure to read bytes
    int offset;             // Offset in the img.archt

    // Wall Brick 1
    offset = tomentdatapack.IMGArch.tocOffset + (tomentdatapack.IMGArch.toc[IMG_ID_W_1].startingOffset);
    sdlWops = SDL_RWFromConstMem((byte*)tomentdatapack.IMGArch.buffer+offset, tomentdatapack.IMGArch.toc[IMG_ID_W_1].size);
    temp1 = SDL_LoadBMP_RW(sdlWops, SDL_TRUE);
    if(D_CheckTextureLoaded(temp1, IMG_ID_W_1))
        tomentdatapack.textures[TEXTURE_WallBrick1]->texture = SDL_ConvertSurface(temp1, win_surface->format, SDL_TEXTUREACCESS_TARGET);
    else
        tomentdatapack.textures[TEXTURE_WallBrick1]->texture = tomentdatapack.enginesDefaults[EDEFAULT_1]->texture;
    SDL_FreeSurface(temp1);

    // Wall Brick 1
    offset = tomentdatapack.IMGArch.tocOffset + (tomentdatapack.IMGArch.toc[IMG_ID_W_1Alt].startingOffset);
    sdlWops = SDL_RWFromConstMem((byte*)tomentdatapack.IMGArch.buffer+offset, tomentdatapack.IMGArch.toc[IMG_ID_W_1Alt].size);
    temp1 = SDL_LoadBMP_RW(sdlWops, SDL_TRUE);
    if(D_CheckTextureLoaded(temp1, IMG_ID_W_1Alt))
        tomentdatapack.textures[TEXTURE_WallBrick1Dark]->texture = SDL_ConvertSurface(temp1, win_surface->format, SDL_TEXTUREACCESS_TARGET);
    else
        tomentdatapack.textures[TEXTURE_WallBrick1Dark]->texture = tomentdatapack.enginesDefaults[EDEFAULT_1]->texture;
    SDL_FreeSurface(temp1);

    // Floor Brick 1
    offset = tomentdatapack.IMGArch.tocOffset + (tomentdatapack.IMGArch.toc[IMG_ID_F_1].startingOffset);
    sdlWops = SDL_RWFromConstMem((byte*)tomentdatapack.IMGArch.buffer+offset, tomentdatapack.IMGArch.toc[IMG_ID_F_1].size);
    temp1 = SDL_LoadBMP_RW(sdlWops, SDL_TRUE);
    if(D_CheckTextureLoaded(temp1, IMG_ID_F_1))
        tomentdatapack.textures[TEXTURE_FloorBrick1]->texture = SDL_ConvertSurface(temp1, win_surface->format, SDL_TEXTUREACCESS_TARGET);
    else
        tomentdatapack.textures[TEXTURE_FloorBrick1]->texture = tomentdatapack.enginesDefaults[EDEFAULT_1]->texture;
    SDL_FreeSurface(temp1);

    // Ceiling Wood 1
    offset = tomentdatapack.IMGArch.tocOffset + (tomentdatapack.IMGArch.toc[IMG_ID_C_1].startingOffset);
    sdlWops = SDL_RWFromConstMem((byte*)tomentdatapack.IMGArch.buffer+offset, tomentdatapack.IMGArch.toc[IMG_ID_C_1].size);
    temp1 = SDL_LoadBMP_RW(sdlWops, SDL_TRUE);
    if(D_CheckTextureLoaded(temp1, IMG_ID_C_1))
        tomentdatapack.textures[TEXTURE_CeilingWood1]->texture = SDL_ConvertSurface(temp1, win_surface->format, SDL_TEXTUREACCESS_TARGET);
    else
        tomentdatapack.textures[TEXTURE_CeilingWood1]->texture = tomentdatapack.enginesDefaults[EDEFAULT_1]->texture;
    SDL_FreeSurface(temp1);

    // Wall 2
    offset = tomentdatapack.IMGArch.tocOffset + (tomentdatapack.IMGArch.toc[IMG_ID_W_2].startingOffset);
    sdlWops = SDL_RWFromConstMem((byte*)tomentdatapack.IMGArch.buffer+offset, tomentdatapack.IMGArch.toc[IMG_ID_W_2].size);
    temp1 = SDL_LoadBMP_RW(sdlWops, SDL_TRUE);
    if(D_CheckTextureLoaded(temp1, IMG_ID_W_2))
        tomentdatapack.textures[TEXTURE_Wall2]->texture = SDL_ConvertSurface(temp1, win_surface->format, SDL_TEXTUREACCESS_TARGET);
    else
        tomentdatapack.textures[TEXTURE_Wall2]->texture = tomentdatapack.enginesDefaults[EDEFAULT_1]->texture;
    SDL_FreeSurface(temp1);

    // Gate 1
    offset = tomentdatapack.IMGArch.tocOffset + (tomentdatapack.IMGArch.toc[IMG_ID_WD_Gate1].startingOffset);
    sdlWops = SDL_RWFromConstMem((byte*)tomentdatapack.IMGArch.buffer+offset, tomentdatapack.IMGArch.toc[IMG_ID_WD_Gate1].size);
    temp1 = SDL_LoadBMP_RW(sdlWops, SDL_TRUE);
    if(D_CheckTextureLoaded(temp1, IMG_ID_WD_Gate1))
        tomentdatapack.textures[TEXTURE_Gate1]->texture = SDL_ConvertSurface(temp1, win_surface->format, SDL_TEXTUREACCESS_TARGET);
    else
        tomentdatapack.textures[TEXTURE_Gate1]->texture = tomentdatapack.enginesDefaults[EDEFAULT_1]->texture;
    SDL_FreeSurface(temp1);

    // Gate 2
    offset = tomentdatapack.IMGArch.tocOffset + (tomentdatapack.IMGArch.toc[IMG_ID_WD_Gate1Alt].startingOffset);
    sdlWops = SDL_RWFromConstMem((byte*)tomentdatapack.IMGArch.buffer+offset, tomentdatapack.IMGArch.toc[IMG_ID_WD_Gate1Alt].size);
    temp1 = SDL_LoadBMP_RW(sdlWops, SDL_TRUE);
    if(D_CheckTextureLoaded(temp1, IMG_ID_WD_Gate1Alt))
        tomentdatapack.textures[TEXTURE_Gate1Alt]->texture = SDL_ConvertSurface(temp1, win_surface->format, SDL_TEXTUREACCESS_TARGET);
    else
        tomentdatapack.textures[TEXTURE_Gate1Alt]->texture = tomentdatapack.enginesDefaults[EDEFAULT_1]->texture;
    SDL_FreeSurface(temp1);

    // Castle Door
    offset = tomentdatapack.IMGArch.tocOffset + (tomentdatapack.IMGArch.toc[IMG_ID_WT_CASTLE_DOORS].startingOffset);
    sdlWops = SDL_RWFromConstMem((byte*)tomentdatapack.IMGArch.buffer+offset, tomentdatapack.IMGArch.toc[IMG_ID_WT_CASTLE_DOORS].size);
    temp1 = SDL_LoadBMP_RW(sdlWops, SDL_TRUE);
    if(D_CheckTextureLoaded(temp1, IMG_ID_WT_CASTLE_DOORS))
        tomentdatapack.textures[TEXTURE_CastleDoor]->texture = SDL_ConvertSurface(temp1, win_surface->format, SDL_TEXTUREACCESS_TARGET);
    else
        tomentdatapack.textures[TEXTURE_CastleDoor]->texture = tomentdatapack.enginesDefaults[EDEFAULT_1]->texture;
    SDL_FreeSurface(temp1);

    // Wall 1 ladder
    offset = tomentdatapack.IMGArch.tocOffset + (tomentdatapack.IMGArch.toc[IMG_ID_WALL1_LADDER].startingOffset);
    sdlWops = SDL_RWFromConstMem((byte*)tomentdatapack.IMGArch.buffer+offset, tomentdatapack.IMGArch.toc[IMG_ID_WALL1_LADDER].size);
    temp1 = SDL_LoadBMP_RW(sdlWops, SDL_TRUE);
    if(D_CheckTextureLoaded(temp1, IMG_ID_WALL1_LADDER))
        tomentdatapack.textures[TEXTURE_Wall1Ladder]->texture = SDL_ConvertSurface(temp1, win_surface->format, SDL_TEXTUREACCESS_TARGET);
    else
        tomentdatapack.textures[TEXTURE_Wall1Ladder]->texture = tomentdatapack.enginesDefaults[EDEFAULT_1]->texture;
    U_SetBit(&tomentdatapack.textures[TEXTURE_Wall1Ladder]->flags, 0); // Set IsLadder = 1
    SDL_FreeSurface(temp1);

    // Floor brick 2
    offset = tomentdatapack.IMGArch.tocOffset + (tomentdatapack.IMGArch.toc[IMG_ID_F_2].startingOffset);
    sdlWops = SDL_RWFromConstMem((byte*)tomentdatapack.IMGArch.buffer+offset, tomentdatapack.IMGArch.toc[IMG_ID_F_2].size);
    temp1 = SDL_LoadBMP_RW(sdlWops, SDL_TRUE);
    if(D_CheckTextureLoaded(temp1, IMG_ID_F_2))
        tomentdatapack.textures[TEXTURE_FloorBrick2]->texture = SDL_ConvertSurface(temp1, win_surface->format, SDL_TEXTUREACCESS_TARGET);
    else
        tomentdatapack.textures[TEXTURE_FloorBrick2]->texture = tomentdatapack.enginesDefaults[EDEFAULT_1]->texture;
    U_SetBit(&tomentdatapack.textures[TEXTURE_FloorBrick2]->flags, 0); // Set IsLadder = 1
    SDL_FreeSurface(temp1);

    // Floor Dirt 1
    offset = tomentdatapack.IMGArch.tocOffset + (tomentdatapack.IMGArch.toc[IMG_ID_F_DIRT].startingOffset);
    sdlWops = SDL_RWFromConstMem((byte*)tomentdatapack.IMGArch.buffer+offset, tomentdatapack.IMGArch.toc[IMG_ID_F_DIRT].size);
    temp1 = SDL_LoadBMP_RW(sdlWops, SDL_TRUE);
    if(D_CheckTextureLoaded(temp1, IMG_ID_F_DIRT))
        tomentdatapack.textures[TEXTURE_FloorDirt1]->texture = SDL_ConvertSurface(temp1, win_surface->format, SDL_TEXTUREACCESS_TARGET);
    else
        tomentdatapack.textures[TEXTURE_FloorDirt1]->texture = tomentdatapack.enginesDefaults[EDEFAULT_1]->texture;
    U_SetBit(&tomentdatapack.textures[TEXTURE_FloorDirt1]->flags, 0); // Set IsLadder = 1
    SDL_FreeSurface(temp1);

    // Ice
    offset = tomentdatapack.IMGArch.tocOffset + (tomentdatapack.IMGArch.toc[IMG_ID_ICE].startingOffset);
    sdlWops = SDL_RWFromConstMem((byte*)tomentdatapack.IMGArch.buffer+offset, tomentdatapack.IMGArch.toc[IMG_ID_ICE].size);
    temp1 = SDL_LoadBMP_RW(sdlWops, SDL_TRUE);
    if(D_CheckTextureLoaded(temp1, IMG_ID_ICE))
        tomentdatapack.textures[TEXTURE_Ice]->texture = SDL_ConvertSurface(temp1, win_surface->format, SDL_TEXTUREACCESS_TARGET);
    else
        tomentdatapack.textures[TEXTURE_Ice]->texture = tomentdatapack.enginesDefaults[EDEFAULT_1]->texture;
    U_SetBit(&tomentdatapack.textures[TEXTURE_Ice]->flags, 0); // Set IsLadder = 1
    SDL_FreeSurface(temp1);

    // Ice Consacrated
    offset = tomentdatapack.IMGArch.tocOffset + (tomentdatapack.IMGArch.toc[IMG_ID_ICE_CONSACRATED].startingOffset);
    sdlWops = SDL_RWFromConstMem((byte*)tomentdatapack.IMGArch.buffer+offset, tomentdatapack.IMGArch.toc[IMG_ID_ICE_CONSACRATED].size);
    temp1 = SDL_LoadBMP_RW(sdlWops, SDL_TRUE);
    if(D_CheckTextureLoaded(temp1, IMG_ID_ICE_CONSACRATED))
        tomentdatapack.textures[TEXTURE_IceConsacrated]->texture = SDL_ConvertSurface(temp1, win_surface->format, SDL_TEXTUREACCESS_TARGET);
    else
        tomentdatapack.textures[TEXTURE_IceConsacrated]->texture = tomentdatapack.enginesDefaults[EDEFAULT_1]->texture;
    U_SetBit(&tomentdatapack.textures[TEXTURE_IceConsacrated]->flags, 0); // Set IsLadder = 1
    SDL_FreeSurface(temp1);

    // Ice Consacrated
    offset = tomentdatapack.IMGArch.tocOffset + (tomentdatapack.IMGArch.toc[IMG_ID_VIOLET_VOID].startingOffset);
    sdlWops = SDL_RWFromConstMem((byte*)tomentdatapack.IMGArch.buffer+offset, tomentdatapack.IMGArch.toc[IMG_ID_VIOLET_VOID].size);
    temp1 = SDL_LoadBMP_RW(sdlWops, SDL_TRUE);
    if(D_CheckTextureLoaded(temp1, IMG_ID_VIOLET_VOID))
        tomentdatapack.textures[TEXTURE_VioletVoid]->texture = SDL_ConvertSurface(temp1, win_surface->format, SDL_TEXTUREACCESS_TARGET);
    else
        tomentdatapack.textures[TEXTURE_VioletVoid]->texture = tomentdatapack.enginesDefaults[EDEFAULT_1]->texture;
    U_SetBit(&tomentdatapack.textures[TEXTURE_VioletVoid]->flags, 0); // Set IsLadder = 1
    SDL_FreeSurface(temp1);

    // IcyGround
    offset = tomentdatapack.IMGArch.tocOffset + (tomentdatapack.IMGArch.toc[IMG_ID_TEXTURE_ICYGROUND].startingOffset);
    sdlWops = SDL_RWFromConstMem((byte*)tomentdatapack.IMGArch.buffer+offset, tomentdatapack.IMGArch.toc[IMG_ID_TEXTURE_ICYGROUND].size);
    temp1 = SDL_LoadBMP_RW(sdlWops, SDL_TRUE);
    if(D_CheckTextureLoaded(temp1, IMG_ID_TEXTURE_ICYGROUND))
        tomentdatapack.textures[TEXTURE_IcyGround]->texture = SDL_ConvertSurface(temp1, win_surface->format, SDL_TEXTUREACCESS_TARGET);
    else
        tomentdatapack.textures[TEXTURE_IcyGround]->texture = tomentdatapack.enginesDefaults[EDEFAULT_1]->texture;
    U_SetBit(&tomentdatapack.textures[TEXTURE_IcyGround]->flags, 0); // Set IsLadder = 1
    SDL_FreeSurface(temp1);

    // Final sets
    wallBrick1->ID = TEXTURE_WallBrick1;
    wallBrick1Dark->ID = TEXTURE_WallBrick1Dark;
    floorBrick1->ID = TEXTURE_FloorBrick1;
    ceilingWood1->ID = TEXTURE_CeilingWood1;
    wall2->ID = TEXTURE_Wall2;
    gate1->ID = TEXTURE_Gate1;
    gate1Alt->ID = TEXTURE_Gate1Alt;
    castleDoor->ID = TEXTURE_CastleDoor;
    wall1Ladder->ID = TEXTURE_Wall1Ladder;
    floorBrick2->ID = TEXTURE_FloorBrick2;
    floorDirt1->ID = TEXTURE_FloorDirt1;
    ice->ID = TEXTURE_Ice;
    iceConsacrated->ID = TEXTURE_IceConsacrated;
    violetVoid->ID = TEXTURE_VioletVoid;
    icyGround->ID = TEXTURE_IcyGround;
}

void D_InitLoadWalls(void)
{
    // Create Objects
    wallAsset_t* wall = (wallAsset_t*)malloc(sizeof(wallAsset_t));
    wallAsset_t* thinWallHor = (wallAsset_t*)malloc(sizeof(wallAsset_t));
    wallAsset_t* thinWallVer = (wallAsset_t*)malloc(sizeof(wallAsset_t));
    wallAsset_t* doorHor = (wallAsset_t*)malloc(sizeof(wallAsset_t));
    wallAsset_t* doorVer = (wallAsset_t*)malloc(sizeof(wallAsset_t));
    wallAsset_t* wallTriggerChangeMap = (wallAsset_t*)malloc(sizeof(wallAsset_t));
    wallAsset_t* wallLadder = (wallAsset_t*)malloc(sizeof(wallAsset_t));
    wallAsset_t* wallLadderDown = (wallAsset_t*)malloc(sizeof(wallAsset_t));
    wallAsset_t* wallInvisible = (wallAsset_t*)malloc(sizeof(wallAsset_t));

    tomentdatapack.wallsLength = 9; // Set length

    D_InitWallAsset(wall);
    D_InitWallAsset(thinWallHor);
    D_InitWallAsset(thinWallVer);
    D_InitWallAsset(doorHor);
    D_InitWallAsset(doorVer);
    D_InitWallAsset(wallTriggerChangeMap);
    D_InitWallAsset(wallLadder);
    D_InitWallAsset(wallLadderDown);
    D_InitWallAsset(wallInvisible);

    // Put objects in the datapack
    tomentdatapack.walls[W_Wall] = wall;
    tomentdatapack.walls[W_ThinWallHor] = thinWallHor;
    tomentdatapack.walls[W_ThinWallVer] = thinWallVer;
    tomentdatapack.walls[W_DoorHor] = doorHor;
    tomentdatapack.walls[W_DoorVer] = doorVer;
    tomentdatapack.walls[W_WallTriggerChangeMap] = wallTriggerChangeMap;
    tomentdatapack.walls[W_WallLadder] = wallLadder;
    tomentdatapack.walls[W_WallLadderDown] = wallLadderDown;
    tomentdatapack.walls[W_WallInvisible] = wallInvisible;

    // Set Wall
    tomentdatapack.walls[W_Wall]->ID = W_Wall;
    tomentdatapack.walls[W_Wall]->flags = 0;
    tomentdatapack.walls[W_Wall]->Callback = NULL;

    // Set Thin Wall Hor
    tomentdatapack.walls[W_ThinWallHor]->ID = W_Wall;
    U_SetBit(&tomentdatapack.walls[W_ThinWallHor]->flags, 0); // Set Thin Wall bit flag to 1, by not setting the next bit this is horizontal
    tomentdatapack.walls[W_ThinWallHor]->Callback = NULL;

    // Set Thin Wall Ver
    tomentdatapack.walls[W_ThinWallVer]->ID = W_DoorHor;
    U_SetBit(&tomentdatapack.walls[W_ThinWallVer]->flags, 0); // Set Thin Wall bit flag to 1
    U_SetBit(&tomentdatapack.walls[W_ThinWallVer]->flags, 1); // Set Vertical bit flag to 1
    tomentdatapack.walls[W_ThinWallVer]->Callback = NULL;

    // Set DoorHor
    tomentdatapack.walls[W_DoorHor]->ID = W_DoorHor;
    U_SetBit(&tomentdatapack.walls[W_DoorHor]->flags, 0); // Set Thin Wall bit flag to 1, by not setting the next bit this is horizontal
    U_SetBit(&tomentdatapack.walls[W_DoorHor]->flags, 2); // Set Door bit flag to 1
    tomentdatapack.walls[W_DoorHor]->Callback = NULL;

    // Set DoorVer
    tomentdatapack.walls[W_DoorVer]->ID = W_DoorVer;
    U_SetBit(&tomentdatapack.walls[W_DoorVer]->flags, 0); // Set Thin Wall bit flag to 1,
    U_SetBit(&tomentdatapack.walls[W_DoorVer]->flags, 1); // Set Vertical bit flag to 1
    U_SetBit(&tomentdatapack.walls[W_DoorVer]->flags, 2); // Set Door bit flag to 1
    tomentdatapack.walls[W_DoorVer]->Callback = NULL;

    // WD_CastleDoorsLvl2
    tomentdatapack.walls[W_WallTriggerChangeMap]->ID = W_WallTriggerChangeMap;
    U_SetBit(&tomentdatapack.walls[W_WallTriggerChangeMap]->flags, 3); // Set Trigger bit flag to 1
    // Set callback and data because this is a trigger
    tomentdatapack.walls[W_WallTriggerChangeMap]->Callback = D_CallbackChangeMap;

    // Wall Ladder
    tomentdatapack.walls[W_WallLadder]->ID = W_WallLadder;
    U_SetBit(&tomentdatapack.walls[W_WallLadder]->flags, 3); // Set Trigger bit flag to 1
    // Set callback and data because this is a trigger
    tomentdatapack.walls[W_WallLadder]->Callback = D_CallbackLadder;

    // Wall Ladder Down
    tomentdatapack.walls[W_WallLadderDown]->ID = W_WallLadderDown;
    U_SetBit(&tomentdatapack.walls[W_WallLadderDown]->flags, 3); // Set Trigger bit flag to 1
    U_SetBit(&tomentdatapack.walls[W_WallLadderDown]->flags, 4); // Set Invisible bit flag to 1

    // Set callback and data because this is a trigger
    tomentdatapack.walls[W_WallLadderDown]->Callback = D_CallbackLadderDown;

    // Wall Invisible
    tomentdatapack.walls[W_WallInvisible]->ID = W_WallInvisible;
    U_SetBit(&tomentdatapack.walls[W_WallInvisible]->flags, 4); // Set Invisible bit flag to 1
    tomentdatapack.walls[W_WallInvisible]->Callback = NULL;
}


void D_InitLoadSprites(void)
{
    // Create Objects
    object_t* spritesBarrel1 = (object_t*)malloc(sizeof(object_t));
    object_t* spritesCampfire = (object_t*)malloc(sizeof(object_t));
    object_t* aiSkeleton = (object_t*)malloc(sizeof(object_t));
    object_t* spellFireball1 = (object_t*)malloc(sizeof(object_t));
    object_t* pickupAxe = (object_t*)malloc(sizeof(object_t));
    object_t* pickupHealthPotion = (object_t*)malloc(sizeof(object_t));
    object_t* pickupManaPotion = (object_t*)malloc(sizeof(object_t));
    object_t* spellIceDart1 = (object_t*)malloc(sizeof(object_t));
    object_t* tomeFireball1 = (object_t*)malloc(sizeof(object_t));
    object_t* tomeIceDart1 = (object_t*)malloc(sizeof(object_t));
    object_t* table1 = (object_t*)malloc(sizeof(object_t));
    object_t* skullStatic = (object_t*)malloc(sizeof(object_t));
    object_t* aiSkeletonElite = (object_t*)malloc(sizeof(object_t));
    object_t* altarEmpty = (object_t*)malloc(sizeof(object_t));
    object_t* altarHealth = (object_t*)malloc(sizeof(object_t));
    object_t* altarMana = (object_t*)malloc(sizeof(object_t));
    object_t* aiSkeletonBurnt = (object_t*)malloc(sizeof(object_t));
    object_t* pickupGreatsword = (object_t*)malloc(sizeof(object_t));
    object_t* aiSkeletonLord = (object_t*)malloc(sizeof(object_t));
    object_t* achtung = (object_t*)malloc(sizeof(object_t));
    object_t* playerCharacterTank = (object_t*)malloc(sizeof(object_t));
    object_t* playerCharacterHealer = (object_t*)malloc(sizeof(object_t));
    object_t* playerCharacterDPS = (object_t*)malloc(sizeof(object_t));
    object_t* spellConcentratedHeal = (object_t*)malloc(sizeof(object_t));
    object_t* spellSwordProjectile = (object_t*)malloc(sizeof(object_t));
    object_t* aiMorgathulTheKeeper = (object_t*)malloc(sizeof(object_t));
    object_t* morgathulOrb = (object_t*)malloc(sizeof(object_t));
    object_t* aiKroganar = (object_t*)malloc(sizeof(object_t));
    object_t* aiMorgathulCopy = (object_t*)malloc(sizeof(object_t));
    object_t* altarSpellPower = (object_t*)malloc(sizeof(object_t));
    object_t* aiTheFrozenLord = (object_t*)malloc(sizeof(object_t));
    object_t* pickupChangeEnvironmentTheFrozenEnd = (object_t*)malloc(sizeof(object_t));
    object_t* spellIceBlast = (object_t*)malloc(sizeof(object_t));
    object_t* aiFrozenLordsCaster = (object_t*)malloc(sizeof(object_t));

    tomentdatapack.spritesLength = 34; // Set length

    D_InitObject(spritesBarrel1);
    D_InitObject(spritesCampfire);
    D_InitObject(aiSkeleton);
    D_InitObject(spellFireball1);
    D_InitObject(pickupAxe);
    D_InitObject(pickupHealthPotion);
    D_InitObject(pickupManaPotion);
    D_InitObject(spellIceDart1);
    D_InitObject(tomeFireball1);
    D_InitObject(tomeIceDart1);
    D_InitObject(table1);
    D_InitObject(skullStatic);
    D_InitObject(aiSkeletonElite);
    D_InitObject(altarEmpty);
    D_InitObject(altarHealth);
    D_InitObject(altarMana);
    D_InitObject(aiSkeletonBurnt);
    D_InitObject(pickupGreatsword);
    D_InitObject(aiSkeletonLord);
    D_InitObject(achtung);
    D_InitObject(playerCharacterTank);
    D_InitObject(playerCharacterHealer);
    D_InitObject(playerCharacterDPS);
    D_InitObject(spellConcentratedHeal);
    D_InitObject(spellSwordProjectile);
    D_InitObject(aiMorgathulTheKeeper);
    D_InitObject(morgathulOrb);
    D_InitObject(aiKroganar);
    D_InitObject(aiMorgathulCopy);
    D_InitObject(altarSpellPower);
    D_InitObject(aiTheFrozenLord);
    D_InitObject(pickupChangeEnvironmentTheFrozenEnd);
    D_InitObject(spellIceBlast);
    D_InitObject(aiFrozenLordsCaster);

    // Put objects in the datapack
    tomentdatapack.sprites[S_Barrel1] = spritesBarrel1;
    tomentdatapack.sprites[S_Campfire] = spritesCampfire;
    tomentdatapack.sprites[DS_Skeleton] = aiSkeleton;
    tomentdatapack.sprites[S_Fireball1] = spellFireball1;
    tomentdatapack.sprites[S_PickupAxe] = pickupAxe;
    tomentdatapack.sprites[S_PickupHealthPotion] = pickupHealthPotion;
    tomentdatapack.sprites[S_PickupManaPotion] = pickupManaPotion;
    tomentdatapack.sprites[S_IceDart1] = spellIceDart1;
    tomentdatapack.sprites[S_TomeFireball1] = tomeFireball1;
    tomentdatapack.sprites[S_TomeIceDart1] = tomeIceDart1;
    tomentdatapack.sprites[S_Table1] = table1;
    tomentdatapack.sprites[S_SkullStatic] = skullStatic;
    tomentdatapack.sprites[DS_SkeletonElite] = aiSkeletonElite;
    tomentdatapack.sprites[S_AltarEmpty] = altarEmpty;
    tomentdatapack.sprites[S_AltarHealth] = altarHealth;
    tomentdatapack.sprites[S_AltarMana] = altarMana;
    tomentdatapack.sprites[DS_SkeletonBurnt] = aiSkeletonBurnt;
    tomentdatapack.sprites[S_PickupGreatsword] = pickupGreatsword;
    tomentdatapack.sprites[DS_SkeletonLord] = aiSkeletonLord;
    tomentdatapack.sprites[S_Achtung] = achtung;
    tomentdatapack.sprites[DS_PlayerTank] = playerCharacterTank;
    tomentdatapack.sprites[DS_PlayerHealer] = playerCharacterHealer;
    tomentdatapack.sprites[DS_PlayerDPS] = playerCharacterDPS;
    tomentdatapack.sprites[S_ConcentratedHeal] = spellConcentratedHeal;
    tomentdatapack.sprites[S_SwordProjectile] = spellSwordProjectile;
    tomentdatapack.sprites[DS_MorgathulTheKeeper] = aiMorgathulTheKeeper;
    tomentdatapack.sprites[S_MorgathulOrb] = morgathulOrb;
    tomentdatapack.sprites[DS_Kroganar] = aiKroganar;
    tomentdatapack.sprites[DS_MorgathulCopy] = aiMorgathulCopy;
    tomentdatapack.sprites[S_AltarSpellPower] = altarSpellPower;
    tomentdatapack.sprites[DS_TheFrozenLord] = aiTheFrozenLord;
    tomentdatapack.sprites[S_PickupChangeEnvironmentTheFrozenEnd] = pickupChangeEnvironmentTheFrozenEnd;
    tomentdatapack.sprites[S_IceBlast] = spellIceBlast;
    tomentdatapack.sprites[DS_FrozenLordsCaster] = aiFrozenLordsCaster;

    // Fill objects
    // Convert all the surfaces that we will load in the same format as the win_surface
    SDL_Surface *temp1;     // Surface
    SDL_RWops* sdlWops;     // Structure to read bytes
    int offset;             // Offset in the img.archt
    
    // Barrel
    offset = tomentdatapack.IMGArch.tocOffset + (tomentdatapack.IMGArch.toc[IMG_ID_S_Barrel1].startingOffset);
    sdlWops = SDL_RWFromConstMem((byte*)tomentdatapack.IMGArch.buffer+offset, tomentdatapack.IMGArch.toc[IMG_ID_S_Barrel1].size);
    temp1 = SDL_LoadBMP_RW(sdlWops, SDL_TRUE);
    if(D_CheckTextureLoaded(temp1, IMG_ID_S_Barrel1))
        tomentdatapack.sprites[S_Barrel1]->texture = SDL_ConvertSurface(temp1, win_surface->format, SDL_TEXTUREACCESS_TARGET);
    else
        tomentdatapack.sprites[S_Barrel1]->texture = tomentdatapack.enginesDefaults[EDEFAULT_1]->texture;
    U_SetBit(&tomentdatapack.sprites[S_Barrel1]->flags, 0); // Set collision bit flag to 1
    // Sprite-Specific, set the lookup table for the sprite sheets length
    tomentdatapack.spritesSheetsLenghtTable[S_Barrel1] = 0;
    // Callback
    tomentdatapack.sprites[S_Barrel1]->Callback = NULL;
    SDL_FreeSurface(temp1);

    // Campfire
    offset = tomentdatapack.IMGArch.tocOffset + (tomentdatapack.IMGArch.toc[IMG_ID_S_Campfire].startingOffset);
    sdlWops = SDL_RWFromConstMem((byte*)tomentdatapack.IMGArch.buffer+offset, tomentdatapack.IMGArch.toc[IMG_ID_S_Campfire].size);
    temp1 = SDL_LoadBMP_RW(sdlWops, SDL_TRUE);
    if(D_CheckTextureLoaded(temp1, IMG_ID_S_Campfire))
        tomentdatapack.sprites[S_Campfire]->texture = SDL_ConvertSurface(temp1, win_surface->format, SDL_TEXTUREACCESS_TARGET);
    else
        tomentdatapack.sprites[S_Campfire]->texture = tomentdatapack.enginesDefaults[EDEFAULT_1]->texture;
    U_SetBit(&tomentdatapack.sprites[S_Campfire]->flags, 0); // Set collision bit flag to 1
    U_SetBit(&tomentdatapack.sprites[S_Campfire]->flags, 1); // Set animated sprite bit flag to 1
    // Sprite-Specific, set the lookup table for the sprite sheets length
    tomentdatapack.spritesSheetsLenghtTable[S_Campfire] = 4;

    // Callback
    tomentdatapack.sprites[S_Campfire]->Callback = NULL;

    SDL_FreeSurface(temp1);

    // LOAD DYNAMIC
    // AI Skeleton
    offset = tomentdatapack.IMGArch.tocOffset + (tomentdatapack.IMGArch.toc[IMG_ID_AI_SKELETON].startingOffset);
    sdlWops = SDL_RWFromConstMem((byte*)tomentdatapack.IMGArch.buffer+offset, tomentdatapack.IMGArch.toc[IMG_ID_AI_SKELETON].size);
    temp1 = SDL_LoadBMP_RW(sdlWops, SDL_TRUE);
    if(D_CheckTextureLoaded(temp1, IMG_ID_AI_SKELETON))
    {
        tomentdatapack.sprites[DS_Skeleton]->texture = SDL_ConvertSurface(temp1, win_surface->format, SDL_TEXTUREACCESS_TARGET);

        // Load animations as well
        tomentdatapack.sprites[DS_Skeleton]->animations = (objectanimations_t*)malloc(sizeof(objectanimations_t));
        D_InitializeObjectAnim(tomentdatapack.sprites[DS_Skeleton]->animations);
        tomentdatapack.sprites[DS_Skeleton]->animations->belongsTo = tomentdatapack.sprites[DS_Skeleton];

        // Idle = Normal
        tomentdatapack.sprites[DS_Skeleton]->animations->animIdle = SDL_ConvertSurface(temp1, win_surface->format, SDL_TEXTUREACCESS_TARGET);
        tomentdatapack.sprites[DS_Skeleton]->animations->animIdleSheetLength = 0;

        // Skeleton Death
        int animOffset = tomentdatapack.IMGArch.tocOffset + (tomentdatapack.IMGArch.toc[IMG_ID_AI_SKELETON_DEATH].startingOffset);
        SDL_RWops* animSdlWops = SDL_RWFromConstMem((byte*)tomentdatapack.IMGArch.buffer+animOffset, tomentdatapack.IMGArch.toc[IMG_ID_AI_SKELETON_DEATH].size);
        SDL_Surface* animTemp1 = SDL_LoadBMP_RW(animSdlWops, SDL_TRUE);
        tomentdatapack.sprites[DS_Skeleton]->animations->animDie = SDL_ConvertSurface(animTemp1, win_surface->format, SDL_TEXTUREACCESS_TARGET);
        tomentdatapack.sprites[DS_Skeleton]->animations->animDieSheetLength = 4;
        SDL_FreeSurface(animTemp1);

        // Skeleton Attack
        animOffset = tomentdatapack.IMGArch.tocOffset + (tomentdatapack.IMGArch.toc[IMG_ID_AI_SKELETON_ATTACK].startingOffset);
        animSdlWops = SDL_RWFromConstMem((byte*)tomentdatapack.IMGArch.buffer+animOffset, tomentdatapack.IMGArch.toc[IMG_ID_AI_SKELETON_ATTACK].size);
        animTemp1 = SDL_LoadBMP_RW(animSdlWops, SDL_TRUE);
        tomentdatapack.sprites[DS_Skeleton]->animations->animAttack = SDL_ConvertSurface(animTemp1, win_surface->format, SDL_TEXTUREACCESS_TARGET);
        tomentdatapack.sprites[DS_Skeleton]->animations->animAttackSheetLength = 4;
        SDL_FreeSurface(animTemp1);

        // Skeleton Resurrect
        animOffset = tomentdatapack.IMGArch.tocOffset + (tomentdatapack.IMGArch.toc[IMG_ID_AI_SKELETON_RESURRECTION].startingOffset);
        animSdlWops = SDL_RWFromConstMem((byte*)tomentdatapack.IMGArch.buffer+animOffset, tomentdatapack.IMGArch.toc[IMG_ID_AI_SKELETON_RESURRECTION].size);
        animTemp1 = SDL_LoadBMP_RW(animSdlWops, SDL_TRUE);
        tomentdatapack.sprites[DS_Skeleton]->animations->animSpecial1 = SDL_ConvertSurface(animTemp1, win_surface->format, SDL_TEXTUREACCESS_TARGET);
        tomentdatapack.sprites[DS_Skeleton]->animations->animSpecial1SheetLength = 4;
        SDL_FreeSurface(animTemp1);
    }
    else
        tomentdatapack.sprites[DS_Skeleton]->texture = tomentdatapack.enginesDefaults[EDEFAULT_1]->texture;
    U_SetBit(&tomentdatapack.sprites[DS_Skeleton]->flags, 0); // Set collision bit flag to 1
    U_SetBit(&tomentdatapack.sprites[DS_Skeleton]->flags, 2); // Set dynamic bit flag to 1

    // Callback
    tomentdatapack.sprites[DS_Skeleton]->Callback = NULL;

    SDL_FreeSurface(temp1);

    // Spell Fireball1
    offset = tomentdatapack.IMGArch.tocOffset + (tomentdatapack.IMGArch.toc[IMG_ID_SPELL_FIREBALL1].startingOffset);
    sdlWops = SDL_RWFromConstMem((byte*)tomentdatapack.IMGArch.buffer+offset, tomentdatapack.IMGArch.toc[IMG_ID_SPELL_FIREBALL1].size);
    temp1 = SDL_LoadBMP_RW(sdlWops, SDL_TRUE);
    if(D_CheckTextureLoaded(temp1, IMG_ID_SPELL_FIREBALL1))
    {
        tomentdatapack.sprites[S_Fireball1]->texture = SDL_ConvertSurface(temp1, win_surface->format, SDL_TEXTUREACCESS_TARGET);

        // Load animations as well
        tomentdatapack.sprites[S_Fireball1]->animations = (objectanimations_t*)malloc(sizeof(objectanimations_t));
        D_InitializeObjectAnim(tomentdatapack.sprites[S_Fireball1]->animations);
        tomentdatapack.sprites[S_Fireball1]->animations->belongsTo = tomentdatapack.sprites[S_Fireball1];

        // Idle = Normal
        tomentdatapack.sprites[S_Fireball1]->animations->animIdle = SDL_ConvertSurface(temp1, win_surface->format, SDL_TEXTUREACCESS_TARGET);
        tomentdatapack.sprites[S_Fireball1]->animations->animIdleSheetLength = 4;

        // Skeleton Death
        int animOffset = tomentdatapack.IMGArch.tocOffset + (tomentdatapack.IMGArch.toc[IMG_ID_SPELL_FIREBALL1_EXPLOSION].startingOffset);
        SDL_RWops* animSdlWops = SDL_RWFromConstMem((byte*)tomentdatapack.IMGArch.buffer+animOffset, tomentdatapack.IMGArch.toc[IMG_ID_SPELL_FIREBALL1_EXPLOSION].size);
        SDL_Surface* animTemp1 = SDL_LoadBMP_RW(animSdlWops, SDL_TRUE);
        tomentdatapack.sprites[S_Fireball1]->animations->animDie = SDL_ConvertSurface(animTemp1, win_surface->format, SDL_TEXTUREACCESS_TARGET);
        tomentdatapack.sprites[S_Fireball1]->animations->animDieSheetLength = 4;

        SDL_FreeSurface(animTemp1);
    }
    else
        tomentdatapack.sprites[S_Fireball1]->texture = tomentdatapack.enginesDefaults[EDEFAULT_1]->texture;
    U_SetBit(&tomentdatapack.sprites[S_Fireball1]->flags, 0); // Set collision bit flag to 1
    U_SetBit(&tomentdatapack.sprites[S_Fireball1]->flags, 1); // Set animated sprite bit flag to 1
    // Sprite-Specific, set the lookup table for the sprite sheets length
    tomentdatapack.spritesSheetsLenghtTable[S_Fireball1] = 4;

    // Callback
    tomentdatapack.sprites[S_Fireball1]->Callback = NULL;
    SDL_FreeSurface(temp1);

    // Pickup axe
    offset = tomentdatapack.IMGArch.tocOffset + (tomentdatapack.IMGArch.toc[IMG_ID_PICKUP_AXE].startingOffset);
    sdlWops = SDL_RWFromConstMem((byte*)tomentdatapack.IMGArch.buffer+offset, tomentdatapack.IMGArch.toc[IMG_ID_PICKUP_AXE].size);
    temp1 = SDL_LoadBMP_RW(sdlWops, SDL_TRUE);
    if(D_CheckTextureLoaded(temp1, IMG_ID_PICKUP_AXE))
        tomentdatapack.sprites[S_PickupAxe]->texture = SDL_ConvertSurface(temp1, win_surface->format, SDL_TEXTUREACCESS_TARGET);
    else
        tomentdatapack.sprites[S_PickupAxe]->texture = tomentdatapack.enginesDefaults[EDEFAULT_1]->texture;
    U_SetBit(&tomentdatapack.sprites[S_PickupAxe]->flags, 0); // Set collision bit flag to 1
    U_SetBit(&tomentdatapack.sprites[S_PickupAxe]->flags, 1); // Set animated sprite bit flag to 1
    // Sprite-Specific, set the lookup table for the sprite sheets length
    tomentdatapack.spritesSheetsLenghtTable[S_PickupAxe] = 6;

    // Callback
    tomentdatapack.sprites[S_PickupAxe]->Callback = D_CallbackPickup;
    tomentdatapack.sprites[S_PickupAxe]->data = "WEAPON_AXE";
    SDL_FreeSurface(temp1);

    // Pickup Health Potion
    offset = tomentdatapack.IMGArch.tocOffset + (tomentdatapack.IMGArch.toc[IMG_ID_PICKUP_HEALTH_POTION].startingOffset);
    sdlWops = SDL_RWFromConstMem((byte*)tomentdatapack.IMGArch.buffer+offset, tomentdatapack.IMGArch.toc[IMG_ID_PICKUP_HEALTH_POTION].size);
    temp1 = SDL_LoadBMP_RW(sdlWops, SDL_TRUE);
    if(D_CheckTextureLoaded(temp1, IMG_ID_PICKUP_HEALTH_POTION))
        tomentdatapack.sprites[S_PickupHealthPotion]->texture = SDL_ConvertSurface(temp1, win_surface->format, SDL_TEXTUREACCESS_TARGET);
    else
        tomentdatapack.sprites[S_PickupHealthPotion]->texture = tomentdatapack.enginesDefaults[EDEFAULT_1]->texture;

    U_SetBit(&tomentdatapack.sprites[S_PickupHealthPotion]->flags, 3); // Auto call callback upon player's collision (to pickup the potion by touching it)

    // Callback
    tomentdatapack.sprites[S_PickupHealthPotion]->Callback = D_CallbackPickup;
    tomentdatapack.sprites[S_PickupHealthPotion]->data = "PICKUP_HEALTH";
    SDL_FreeSurface(temp1);

    // Pickup Mana Potion
    offset = tomentdatapack.IMGArch.tocOffset + (tomentdatapack.IMGArch.toc[IMG_ID_PICKUP_MANA_POTION].startingOffset);
    sdlWops = SDL_RWFromConstMem((byte*)tomentdatapack.IMGArch.buffer+offset, tomentdatapack.IMGArch.toc[IMG_ID_PICKUP_MANA_POTION].size);
    temp1 = SDL_LoadBMP_RW(sdlWops, SDL_TRUE);
    if(D_CheckTextureLoaded(temp1, IMG_ID_PICKUP_MANA_POTION))
        tomentdatapack.sprites[S_PickupManaPotion]->texture = SDL_ConvertSurface(temp1, win_surface->format, SDL_TEXTUREACCESS_TARGET);
    else
        tomentdatapack.sprites[S_PickupManaPotion]->texture = tomentdatapack.enginesDefaults[EDEFAULT_1]->texture;

    U_SetBit(&tomentdatapack.sprites[S_PickupManaPotion]->flags, 3); // Auto call callback upon player's collision (to pickup the potion by touching it)

    // Callback
    tomentdatapack.sprites[S_PickupManaPotion]->Callback = D_CallbackPickup;
    tomentdatapack.sprites[S_PickupManaPotion]->data = "PICKUP_MANA";
    SDL_FreeSurface(temp1);

    // Spell IceDart1
    offset = tomentdatapack.IMGArch.tocOffset + (tomentdatapack.IMGArch.toc[IMG_ID_SPELL_ICEDART1].startingOffset);
    sdlWops = SDL_RWFromConstMem((byte*)tomentdatapack.IMGArch.buffer+offset, tomentdatapack.IMGArch.toc[IMG_ID_SPELL_ICEDART1].size);
    temp1 = SDL_LoadBMP_RW(sdlWops, SDL_TRUE);
    if(D_CheckTextureLoaded(temp1, IMG_ID_SPELL_ICEDART1))
    {
        tomentdatapack.sprites[S_IceDart1]->texture = SDL_ConvertSurface(temp1, win_surface->format, SDL_TEXTUREACCESS_TARGET);

        // Load animations as well
        tomentdatapack.sprites[S_IceDart1]->animations = (objectanimations_t*)malloc(sizeof(objectanimations_t));
        D_InitializeObjectAnim(tomentdatapack.sprites[S_IceDart1]->animations);
        tomentdatapack.sprites[S_IceDart1]->animations->belongsTo = tomentdatapack.sprites[S_IceDart1];

        // Idle = Normal
        tomentdatapack.sprites[S_IceDart1]->animations->animIdle = SDL_ConvertSurface(temp1, win_surface->format, SDL_TEXTUREACCESS_TARGET);
        tomentdatapack.sprites[S_IceDart1]->animations->animIdleSheetLength = 6;

        // Skeleton Death
        int animOffset = tomentdatapack.IMGArch.tocOffset + (tomentdatapack.IMGArch.toc[IMG_ID_SPELL_ICEDART_EXPLOSION].startingOffset);
        SDL_RWops* animSdlWops = SDL_RWFromConstMem((byte*)tomentdatapack.IMGArch.buffer+animOffset, tomentdatapack.IMGArch.toc[IMG_ID_SPELL_ICEDART_EXPLOSION].size);
        SDL_Surface* animTemp1 = SDL_LoadBMP_RW(animSdlWops, SDL_TRUE);
        tomentdatapack.sprites[S_IceDart1]->animations->animDie = SDL_ConvertSurface(animTemp1, win_surface->format, SDL_TEXTUREACCESS_TARGET);
        tomentdatapack.sprites[S_IceDart1]->animations->animDieSheetLength = 5;

        SDL_FreeSurface(animTemp1);
    }
    else
        tomentdatapack.sprites[S_IceDart1]->texture = tomentdatapack.enginesDefaults[EDEFAULT_1]->texture;
    U_SetBit(&tomentdatapack.sprites[S_IceDart1]->flags, 0); // Set collision bit flag to 1
    U_SetBit(&tomentdatapack.sprites[S_IceDart1]->flags, 1); // Set animated sprite bit flag to 1
    // Sprite-Specific, set the lookup table for the sprite sheets length
    tomentdatapack.spritesSheetsLenghtTable[S_IceDart1] = 6;

    // Callback
    tomentdatapack.sprites[S_IceDart1]->Callback = NULL;
    SDL_FreeSurface(temp1);

    // Pickup Tome Fireball1
    offset = tomentdatapack.IMGArch.tocOffset + (tomentdatapack.IMGArch.toc[IMG_ID_TOME_FIREBALL01].startingOffset);
    sdlWops = SDL_RWFromConstMem((byte*)tomentdatapack.IMGArch.buffer+offset, tomentdatapack.IMGArch.toc[IMG_ID_TOME_FIREBALL01].size);
    temp1 = SDL_LoadBMP_RW(sdlWops, SDL_TRUE);
    if(D_CheckTextureLoaded(temp1, IMG_ID_TOME_FIREBALL01))
        tomentdatapack.sprites[S_TomeFireball1]->texture = SDL_ConvertSurface(temp1, win_surface->format, SDL_TEXTUREACCESS_TARGET);
    else
        tomentdatapack.sprites[S_TomeFireball1]->texture = tomentdatapack.enginesDefaults[EDEFAULT_1]->texture;
    U_SetBit(&tomentdatapack.sprites[S_TomeFireball1]->flags, 0); // Set collision bit flag to 1
    U_SetBit(&tomentdatapack.sprites[S_TomeFireball1]->flags, 1); // Set animated sprite bit flag to 1
    // Sprite-Specific, set the lookup table for the sprite sheets length
    tomentdatapack.spritesSheetsLenghtTable[S_TomeFireball1] = 7;

    // Callback
    tomentdatapack.sprites[S_TomeFireball1]->Callback = D_CallbackPickup;
    tomentdatapack.sprites[S_TomeFireball1]->data = "TOME_FIREBALL";
    SDL_FreeSurface(temp1);

    // Pickup Tome IceDart1
    offset = tomentdatapack.IMGArch.tocOffset + (tomentdatapack.IMGArch.toc[IMG_ID_TOME_ICEDART01].startingOffset);
    sdlWops = SDL_RWFromConstMem((byte*)tomentdatapack.IMGArch.buffer+offset, tomentdatapack.IMGArch.toc[IMG_ID_TOME_ICEDART01].size);
    temp1 = SDL_LoadBMP_RW(sdlWops, SDL_TRUE);
    if(D_CheckTextureLoaded(temp1, IMG_ID_TOME_ICEDART01))
        tomentdatapack.sprites[S_TomeIceDart1]->texture = SDL_ConvertSurface(temp1, win_surface->format, SDL_TEXTUREACCESS_TARGET);
    else
        tomentdatapack.sprites[S_TomeIceDart1]->texture = tomentdatapack.enginesDefaults[EDEFAULT_1]->texture;
    U_SetBit(&tomentdatapack.sprites[S_TomeIceDart1]->flags, 0); // Set collision bit flag to 1
    U_SetBit(&tomentdatapack.sprites[S_TomeIceDart1]->flags, 1); // Set animated sprite bit flag to 1
    // Sprite-Specific, set the lookup table for the sprite sheets length
    tomentdatapack.spritesSheetsLenghtTable[S_TomeIceDart1] = 7;

    // Callback
    tomentdatapack.sprites[S_TomeIceDart1]->Callback = D_CallbackPickup;
    tomentdatapack.sprites[S_TomeIceDart1]->data = "TOME_ICEDART";
    SDL_FreeSurface(temp1);

    // Table1
    offset = tomentdatapack.IMGArch.tocOffset + (tomentdatapack.IMGArch.toc[IMG_ID_S_Table1].startingOffset);
    sdlWops = SDL_RWFromConstMem((byte*)tomentdatapack.IMGArch.buffer+offset, tomentdatapack.IMGArch.toc[IMG_ID_S_Table1].size);
    temp1 = SDL_LoadBMP_RW(sdlWops, SDL_TRUE);
    if(D_CheckTextureLoaded(temp1, IMG_ID_S_Table1))
        tomentdatapack.sprites[S_Table1]->texture = SDL_ConvertSurface(temp1, win_surface->format, SDL_TEXTUREACCESS_TARGET);
    else
        tomentdatapack.sprites[S_Table1]->texture = tomentdatapack.enginesDefaults[EDEFAULT_1]->texture;
    U_SetBit(&tomentdatapack.sprites[S_Table1]->flags, 0); // Set collision bit flag to 1
    // Sprite-Specific, set the lookup table for the sprite sheets length
    tomentdatapack.spritesSheetsLenghtTable[S_Table1] = 0;
    // Callback
    tomentdatapack.sprites[S_Table1]->Callback = NULL;
    SDL_FreeSurface(temp1);

    // Static Skull
    offset = tomentdatapack.IMGArch.tocOffset + (tomentdatapack.IMGArch.toc[IMG_ID_S_SKULL_STATIC].startingOffset);
    sdlWops = SDL_RWFromConstMem((byte*)tomentdatapack.IMGArch.buffer+offset, tomentdatapack.IMGArch.toc[IMG_ID_S_SKULL_STATIC].size);
    temp1 = SDL_LoadBMP_RW(sdlWops, SDL_TRUE);
    if(D_CheckTextureLoaded(temp1, IMG_ID_S_SKULL_STATIC))
        tomentdatapack.sprites[S_SkullStatic]->texture = SDL_ConvertSurface(temp1, win_surface->format, SDL_TEXTUREACCESS_TARGET);
    else
        tomentdatapack.sprites[S_SkullStatic]->texture = tomentdatapack.enginesDefaults[EDEFAULT_1]->texture;
    U_SetBit(&tomentdatapack.sprites[S_SkullStatic]->flags, 0); // Set collision bit flag to 1
    // Sprite-Specific, set the lookup table for the sprite sheets length
    tomentdatapack.spritesSheetsLenghtTable[S_SkullStatic] = 0;
    // Callback
    tomentdatapack.sprites[S_SkullStatic]->Callback = NULL;
    SDL_FreeSurface(temp1);

    // LOAD DYNAMIC
    // AI Skeleton
    offset = tomentdatapack.IMGArch.tocOffset + (tomentdatapack.IMGArch.toc[IMG_ID_AI_SKELETON_ELITE].startingOffset);
    sdlWops = SDL_RWFromConstMem((byte*)tomentdatapack.IMGArch.buffer+offset, tomentdatapack.IMGArch.toc[IMG_ID_AI_SKELETON_ELITE].size);
    temp1 = SDL_LoadBMP_RW(sdlWops, SDL_TRUE);
    if(D_CheckTextureLoaded(temp1, IMG_ID_AI_SKELETON_ELITE))
    {
        tomentdatapack.sprites[DS_SkeletonElite]->texture = SDL_ConvertSurface(temp1, win_surface->format, SDL_TEXTUREACCESS_TARGET);

        // Load animations as well
        tomentdatapack.sprites[DS_SkeletonElite]->animations = (objectanimations_t*)malloc(sizeof(objectanimations_t));
        D_InitializeObjectAnim(tomentdatapack.sprites[DS_SkeletonElite]->animations);
        tomentdatapack.sprites[DS_SkeletonElite]->animations->belongsTo = tomentdatapack.sprites[DS_SkeletonElite];

        // Idle = Normal
        tomentdatapack.sprites[DS_SkeletonElite]->animations->animIdle = SDL_ConvertSurface(temp1, win_surface->format, SDL_TEXTUREACCESS_TARGET);
        tomentdatapack.sprites[DS_SkeletonElite]->animations->animIdleSheetLength = 0;

        // Skeleton Death
        int animOffset = tomentdatapack.IMGArch.tocOffset + (tomentdatapack.IMGArch.toc[IMG_ID_AI_SKELETON_ELITE_DEATH].startingOffset);
        SDL_RWops* animSdlWops = SDL_RWFromConstMem((byte*)tomentdatapack.IMGArch.buffer+animOffset, tomentdatapack.IMGArch.toc[IMG_ID_AI_SKELETON_ELITE_DEATH].size);
        SDL_Surface* animTemp1 = SDL_LoadBMP_RW(animSdlWops, SDL_TRUE);
        tomentdatapack.sprites[DS_SkeletonElite]->animations->animDie = SDL_ConvertSurface(animTemp1, win_surface->format, SDL_TEXTUREACCESS_TARGET);
        tomentdatapack.sprites[DS_SkeletonElite]->animations->animDieSheetLength = 4;
        SDL_FreeSurface(animTemp1);

        // Skeleton Attack
        animOffset = tomentdatapack.IMGArch.tocOffset + (tomentdatapack.IMGArch.toc[IMG_ID_AI_SKELETON_ELITE_ATTACK].startingOffset);
        animSdlWops = SDL_RWFromConstMem((byte*)tomentdatapack.IMGArch.buffer+animOffset, tomentdatapack.IMGArch.toc[IMG_ID_AI_SKELETON_ELITE_ATTACK].size);
        animTemp1 = SDL_LoadBMP_RW(animSdlWops, SDL_TRUE);
        tomentdatapack.sprites[DS_SkeletonElite]->animations->animAttack = SDL_ConvertSurface(animTemp1, win_surface->format, SDL_TEXTUREACCESS_TARGET);
        tomentdatapack.sprites[DS_SkeletonElite]->animations->animAttackSheetLength = 4;
        tomentdatapack.sprites[DS_SkeletonElite]->animations->animAttackSpeedModifier = 25;
        SDL_FreeSurface(animTemp1);

        // Skeleton Resurrect
        animOffset = tomentdatapack.IMGArch.tocOffset + (tomentdatapack.IMGArch.toc[IMG_ID_SKELETON_ELITE_RESURRECTION].startingOffset);
        animSdlWops = SDL_RWFromConstMem((byte*)tomentdatapack.IMGArch.buffer+animOffset, tomentdatapack.IMGArch.toc[IMG_ID_SKELETON_ELITE_RESURRECTION].size);
        animTemp1 = SDL_LoadBMP_RW(animSdlWops, SDL_TRUE);
        tomentdatapack.sprites[DS_SkeletonElite]->animations->animSpecial1 = SDL_ConvertSurface(animTemp1, win_surface->format, SDL_TEXTUREACCESS_TARGET);
        tomentdatapack.sprites[DS_SkeletonElite]->animations->animSpecial1SheetLength = 4;
        SDL_FreeSurface(animTemp1);

    }
    else
        tomentdatapack.sprites[DS_SkeletonElite]->texture = tomentdatapack.enginesDefaults[EDEFAULT_1]->texture;
    U_SetBit(&tomentdatapack.sprites[DS_SkeletonElite]->flags, 0); // Set collision bit flag to 1
    U_SetBit(&tomentdatapack.sprites[DS_SkeletonElite]->flags, 2); // Set dynamic bit flag to 1

    // Callback
    tomentdatapack.sprites[DS_SkeletonElite]->Callback = NULL;
    SDL_FreeSurface(temp1);

    // Altar Empty
    offset = tomentdatapack.IMGArch.tocOffset + (tomentdatapack.IMGArch.toc[IMG_ID_S_ALTAR_EMPTY].startingOffset);
    sdlWops = SDL_RWFromConstMem((byte*)tomentdatapack.IMGArch.buffer+offset, tomentdatapack.IMGArch.toc[IMG_ID_S_ALTAR_EMPTY].size);
    temp1 = SDL_LoadBMP_RW(sdlWops, SDL_TRUE);
    if(D_CheckTextureLoaded(temp1, IMG_ID_S_ALTAR_EMPTY))
        tomentdatapack.sprites[S_AltarEmpty]->texture = SDL_ConvertSurface(temp1, win_surface->format, SDL_TEXTUREACCESS_TARGET);
    else
        tomentdatapack.sprites[S_AltarEmpty]->texture = tomentdatapack.enginesDefaults[EDEFAULT_1]->texture;
    U_SetBit(&tomentdatapack.sprites[S_AltarEmpty]->flags, 0); // Set collision bit flag to 1
    // Sprite-Specific, set the lookup table for the sprite sheets length
    tomentdatapack.spritesSheetsLenghtTable[S_AltarEmpty] = 0;
    // Callback
    tomentdatapack.sprites[S_AltarEmpty]->Callback = NULL;
    SDL_FreeSurface(temp1);

    // Altar Health
    offset = tomentdatapack.IMGArch.tocOffset + (tomentdatapack.IMGArch.toc[IMG_ID_S_ALTAR_HEALTH].startingOffset);
    sdlWops = SDL_RWFromConstMem((byte*)tomentdatapack.IMGArch.buffer+offset, tomentdatapack.IMGArch.toc[IMG_ID_S_ALTAR_HEALTH].size);
    temp1 = SDL_LoadBMP_RW(sdlWops, SDL_TRUE);
    if(D_CheckTextureLoaded(temp1, IMG_ID_S_ALTAR_HEALTH))
        tomentdatapack.sprites[S_AltarHealth]->texture = SDL_ConvertSurface(temp1, win_surface->format, SDL_TEXTUREACCESS_TARGET);
    else
        tomentdatapack.sprites[S_AltarHealth]->texture = tomentdatapack.enginesDefaults[EDEFAULT_1]->texture;
    U_SetBit(&tomentdatapack.sprites[S_AltarHealth]->flags, 0); // Set collision bit flag to 1
    // Sprite-Specific, set the lookup table for the sprite sheets length
    tomentdatapack.spritesSheetsLenghtTable[S_AltarHealth] = 0;

    // Callback
    tomentdatapack.sprites[S_AltarHealth]->Callback = D_CallbackUseAltar;
    tomentdatapack.sprites[S_AltarHealth]->data = "HEALTH";
    SDL_FreeSurface(temp1);

    // Altar Mana
    offset = tomentdatapack.IMGArch.tocOffset + (tomentdatapack.IMGArch.toc[IMG_ID_S_ALTAR_MANA].startingOffset);
    sdlWops = SDL_RWFromConstMem((byte*)tomentdatapack.IMGArch.buffer+offset, tomentdatapack.IMGArch.toc[IMG_ID_S_ALTAR_MANA].size);
    temp1 = SDL_LoadBMP_RW(sdlWops, SDL_TRUE);
    if(D_CheckTextureLoaded(temp1, IMG_ID_S_ALTAR_MANA))
        tomentdatapack.sprites[S_AltarMana]->texture = SDL_ConvertSurface(temp1, win_surface->format, SDL_TEXTUREACCESS_TARGET);
    else
        tomentdatapack.sprites[S_AltarMana]->texture = tomentdatapack.enginesDefaults[EDEFAULT_1]->texture;
    U_SetBit(&tomentdatapack.sprites[S_AltarMana]->flags, 0); // Set collision bit flag to 1
    // Sprite-Specific, set the lookup table for the sprite sheets length
    tomentdatapack.spritesSheetsLenghtTable[S_AltarMana] = 0;

    // Callback
    tomentdatapack.sprites[S_AltarMana]->Callback = D_CallbackUseAltar;
    tomentdatapack.sprites[S_AltarMana]->data = "MANA";
    SDL_FreeSurface(temp1);

    // LOAD DYNAMIC
    // AI SkeletonBurnt
    offset = tomentdatapack.IMGArch.tocOffset + (tomentdatapack.IMGArch.toc[IMG_ID_AI_SKELETON_BURNT].startingOffset);
    sdlWops = SDL_RWFromConstMem((byte*)tomentdatapack.IMGArch.buffer+offset, tomentdatapack.IMGArch.toc[IMG_ID_AI_SKELETON_BURNT].size);
    temp1 = SDL_LoadBMP_RW(sdlWops, SDL_TRUE);
    if(D_CheckTextureLoaded(temp1, IMG_ID_AI_SKELETON_BURNT))
    {
        tomentdatapack.sprites[DS_SkeletonBurnt]->texture = SDL_ConvertSurface(temp1, win_surface->format, SDL_TEXTUREACCESS_TARGET);

        // Load animations as well
        tomentdatapack.sprites[DS_SkeletonBurnt]->animations = (objectanimations_t*)malloc(sizeof(objectanimations_t));
        D_InitializeObjectAnim(tomentdatapack.sprites[DS_SkeletonBurnt]->animations);
        tomentdatapack.sprites[DS_SkeletonBurnt]->animations->belongsTo = tomentdatapack.sprites[DS_SkeletonBurnt];

        // Idle = Normal
        tomentdatapack.sprites[DS_SkeletonBurnt]->animations->animIdle = SDL_ConvertSurface(temp1, win_surface->format, SDL_TEXTUREACCESS_TARGET);
        tomentdatapack.sprites[DS_SkeletonBurnt]->animations->animIdleSheetLength = 0;

        // Skeleton Death
        int animOffset = tomentdatapack.IMGArch.tocOffset + (tomentdatapack.IMGArch.toc[IMG_ID_AI_SKELETON_BURNT_DEATH].startingOffset);
        SDL_RWops* animSdlWops = SDL_RWFromConstMem((byte*)tomentdatapack.IMGArch.buffer+animOffset, tomentdatapack.IMGArch.toc[IMG_ID_AI_SKELETON_BURNT_DEATH].size);
        SDL_Surface* animTemp1 = SDL_LoadBMP_RW(animSdlWops, SDL_TRUE);
        tomentdatapack.sprites[DS_SkeletonBurnt]->animations->animDie = SDL_ConvertSurface(animTemp1, win_surface->format, SDL_TEXTUREACCESS_TARGET);
        tomentdatapack.sprites[DS_SkeletonBurnt]->animations->animDieSheetLength = 4;
        SDL_FreeSurface(animTemp1);

        // Skeleton Attack
        animOffset = tomentdatapack.IMGArch.tocOffset + (tomentdatapack.IMGArch.toc[IMG_ID_AI_SKELETON_BURNT_ATTACK].startingOffset);
        animSdlWops = SDL_RWFromConstMem((byte*)tomentdatapack.IMGArch.buffer+animOffset, tomentdatapack.IMGArch.toc[IMG_ID_AI_SKELETON_BURNT_ATTACK].size);
        animTemp1 = SDL_LoadBMP_RW(animSdlWops, SDL_TRUE);
        tomentdatapack.sprites[DS_SkeletonBurnt]->animations->animAttack = SDL_ConvertSurface(animTemp1, win_surface->format, SDL_TEXTUREACCESS_TARGET);
        tomentdatapack.sprites[DS_SkeletonBurnt]->animations->animAttackSheetLength = 6;
        SDL_FreeSurface(animTemp1);

    }
    else
        tomentdatapack.sprites[DS_SkeletonBurnt]->texture = tomentdatapack.enginesDefaults[EDEFAULT_1]->texture;
    U_SetBit(&tomentdatapack.sprites[DS_SkeletonBurnt]->flags, 0); // Set collision bit flag to 1
    U_SetBit(&tomentdatapack.sprites[DS_SkeletonBurnt]->flags, 2); // Set dynamic bit flag to 1

    // Callback
    tomentdatapack.sprites[DS_SkeletonBurnt]->Callback = NULL;
    SDL_FreeSurface(temp1);

    // Pickup greatsword
    offset = tomentdatapack.IMGArch.tocOffset + (tomentdatapack.IMGArch.toc[IMG_ID_PIKCUP_GREATSWORD].startingOffset);
    sdlWops = SDL_RWFromConstMem((byte*)tomentdatapack.IMGArch.buffer+offset, tomentdatapack.IMGArch.toc[IMG_ID_PIKCUP_GREATSWORD].size);
    temp1 = SDL_LoadBMP_RW(sdlWops, SDL_TRUE);
    if(D_CheckTextureLoaded(temp1, IMG_ID_PIKCUP_GREATSWORD))
        tomentdatapack.sprites[S_PickupGreatsword]->texture = SDL_ConvertSurface(temp1, win_surface->format, SDL_TEXTUREACCESS_TARGET);
    else
        tomentdatapack.sprites[S_PickupGreatsword]->texture = tomentdatapack.enginesDefaults[EDEFAULT_1]->texture;
    U_SetBit(&tomentdatapack.sprites[S_PickupGreatsword]->flags, 0); // Set collision bit flag to 1
    U_SetBit(&tomentdatapack.sprites[S_PickupGreatsword]->flags, 1); // Set animated sprite bit flag to 1
    // Sprite-Specific, set the lookup table for the sprite sheets length
    tomentdatapack.spritesSheetsLenghtTable[S_PickupGreatsword] = 6;

    // Callback
    tomentdatapack.sprites[S_PickupGreatsword]->Callback = D_CallbackPickup;
    tomentdatapack.sprites[S_PickupGreatsword]->data = "WEAPON_GREATSWORD";
    SDL_FreeSurface(temp1);

    // LOAD DYNAMIC
    // AI Skeleton Lord
    offset = tomentdatapack.IMGArch.tocOffset + (tomentdatapack.IMGArch.toc[IMG_ID_SKELETON_LORD].startingOffset);
    sdlWops = SDL_RWFromConstMem((byte*)tomentdatapack.IMGArch.buffer+offset, tomentdatapack.IMGArch.toc[IMG_ID_SKELETON_LORD].size);
    temp1 = SDL_LoadBMP_RW(sdlWops, SDL_TRUE);
    if(D_CheckTextureLoaded(temp1, IMG_ID_SKELETON_LORD))
    {
        tomentdatapack.sprites[DS_SkeletonLord]->texture = SDL_ConvertSurface(temp1, win_surface->format, SDL_TEXTUREACCESS_TARGET);

        // Load animations as well
        tomentdatapack.sprites[DS_SkeletonLord]->animations = (objectanimations_t*)malloc(sizeof(objectanimations_t));
        D_InitializeObjectAnim(tomentdatapack.sprites[DS_SkeletonLord]->animations);
        tomentdatapack.sprites[DS_SkeletonLord]->animations->belongsTo = tomentdatapack.sprites[DS_SkeletonLord];

        // Idle = Normal
        tomentdatapack.sprites[DS_SkeletonLord]->animations->animIdle = SDL_ConvertSurface(temp1, win_surface->format, SDL_TEXTUREACCESS_TARGET);
        tomentdatapack.sprites[DS_SkeletonLord]->animations->animIdleSheetLength = 0;

        // Skeleton Death
        int animOffset = tomentdatapack.IMGArch.tocOffset + (tomentdatapack.IMGArch.toc[IMG_ID_SKELETON_LORD_DEATH].startingOffset);
        SDL_RWops* animSdlWops = SDL_RWFromConstMem((byte*)tomentdatapack.IMGArch.buffer+animOffset, tomentdatapack.IMGArch.toc[IMG_ID_SKELETON_LORD_DEATH].size);
        SDL_Surface* animTemp1 = SDL_LoadBMP_RW(animSdlWops, SDL_TRUE);
        tomentdatapack.sprites[DS_SkeletonLord]->animations->animDie = SDL_ConvertSurface(animTemp1, win_surface->format, SDL_TEXTUREACCESS_TARGET);
        tomentdatapack.sprites[DS_SkeletonLord]->animations->animDieSheetLength = 4;
        SDL_FreeSurface(animTemp1);

        // Skeleton Attack
        animOffset = tomentdatapack.IMGArch.tocOffset + (tomentdatapack.IMGArch.toc[IMG_ID_SKELETON_LORD_ATTACK_MELEE].startingOffset);
        animSdlWops = SDL_RWFromConstMem((byte*)tomentdatapack.IMGArch.buffer+animOffset, tomentdatapack.IMGArch.toc[IMG_ID_SKELETON_LORD_ATTACK_MELEE].size);
        animTemp1 = SDL_LoadBMP_RW(animSdlWops, SDL_TRUE);
        tomentdatapack.sprites[DS_SkeletonLord]->animations->animAttack = SDL_ConvertSurface(animTemp1, win_surface->format, SDL_TEXTUREACCESS_TARGET);
        tomentdatapack.sprites[DS_SkeletonLord]->animations->animAttackSheetLength = 4;
        SDL_FreeSurface(animTemp1);

        // Skeleton Cast spell
        animOffset = tomentdatapack.IMGArch.tocOffset + (tomentdatapack.IMGArch.toc[IMG_ID_SKELETON_LORD_ATTACK_FIREBALL].startingOffset);
        animSdlWops = SDL_RWFromConstMem((byte*)tomentdatapack.IMGArch.buffer+animOffset, tomentdatapack.IMGArch.toc[IMG_ID_SKELETON_LORD_ATTACK_FIREBALL].size);
        animTemp1 = SDL_LoadBMP_RW(animSdlWops, SDL_TRUE);
        tomentdatapack.sprites[DS_SkeletonLord]->animations->animCastSpell = SDL_ConvertSurface(animTemp1, win_surface->format, SDL_TEXTUREACCESS_TARGET);
        tomentdatapack.sprites[DS_SkeletonLord]->animations->animCastSpellSheetLength = 5;
        SDL_FreeSurface(animTemp1);

        // Skeleton Special: Hell
        animOffset = tomentdatapack.IMGArch.tocOffset + (tomentdatapack.IMGArch.toc[IMG_ID_SKELETON_LORD_SPELL_HELL].startingOffset);
        animSdlWops = SDL_RWFromConstMem((byte*)tomentdatapack.IMGArch.buffer+animOffset, tomentdatapack.IMGArch.toc[IMG_ID_SKELETON_LORD_SPELL_HELL].size);
        animTemp1 = SDL_LoadBMP_RW(animSdlWops, SDL_TRUE);
        tomentdatapack.sprites[DS_SkeletonLord]->animations->animSpecial1 = SDL_ConvertSurface(animTemp1, win_surface->format, SDL_TEXTUREACCESS_TARGET);
        tomentdatapack.sprites[DS_SkeletonLord]->animations->animSpecial1SheetLength = 3;
        SDL_FreeSurface(animTemp1);

        // Skeleton Special: Resurrection
        animOffset = tomentdatapack.IMGArch.tocOffset + (tomentdatapack.IMGArch.toc[IMG_ID_SKELETON_LORD_SPELL_RESURRECTION].startingOffset);
        animSdlWops = SDL_RWFromConstMem((byte*)tomentdatapack.IMGArch.buffer+animOffset, tomentdatapack.IMGArch.toc[IMG_ID_SKELETON_LORD_SPELL_RESURRECTION].size);
        animTemp1 = SDL_LoadBMP_RW(animSdlWops, SDL_TRUE);
        tomentdatapack.sprites[DS_SkeletonLord]->animations->animSpecial2 = SDL_ConvertSurface(animTemp1, win_surface->format, SDL_TEXTUREACCESS_TARGET);
        tomentdatapack.sprites[DS_SkeletonLord]->animations->animSpecial2SheetLength = 3;
        SDL_FreeSurface(animTemp1);

    }
    else
        tomentdatapack.sprites[DS_SkeletonLord]->texture = tomentdatapack.enginesDefaults[EDEFAULT_1]->texture;
    U_SetBit(&tomentdatapack.sprites[DS_SkeletonLord]->flags, 0); // Set collision bit flag to 1
    U_SetBit(&tomentdatapack.sprites[DS_SkeletonLord]->flags, 2); // Set dynamic bit flag to 1

    // Callback
    tomentdatapack.sprites[DS_SkeletonLord]->Callback = NULL;
    SDL_FreeSurface(temp1);

    // Achtung
    offset = tomentdatapack.IMGArch.tocOffset + (tomentdatapack.IMGArch.toc[IMG_ID_ACHTUNG].startingOffset);
    sdlWops = SDL_RWFromConstMem((byte*)tomentdatapack.IMGArch.buffer+offset, tomentdatapack.IMGArch.toc[IMG_ID_ACHTUNG].size);
    temp1 = SDL_LoadBMP_RW(sdlWops, SDL_TRUE);
    if(D_CheckTextureLoaded(temp1, IMG_ID_ACHTUNG))
        tomentdatapack.sprites[S_Achtung]->texture = SDL_ConvertSurface(temp1, win_surface->format, SDL_TEXTUREACCESS_TARGET);
    else
        tomentdatapack.sprites[S_Achtung]->texture = tomentdatapack.enginesDefaults[EDEFAULT_1]->texture;
    U_SetBit(&tomentdatapack.sprites[S_Achtung]->flags, 0); // Set collision bit flag to 1
    U_SetBit(&tomentdatapack.sprites[S_Achtung]->flags, 5); // Set 8 angled sprite

    // Sprite-Specific, set the lookup table for the sprite sheets length
    tomentdatapack.spritesSheetsLenghtTable[S_Achtung] = 0;
    // Callback
    tomentdatapack.sprites[S_Achtung]->Callback = NULL;
    SDL_FreeSurface(temp1);

    // PlayerCharacterTank
    offset = tomentdatapack.IMGArch.tocOffset + (tomentdatapack.IMGArch.toc[IMG_ID_PLAYER_TANK_CHARACTER].startingOffset);
    sdlWops = SDL_RWFromConstMem((byte*)tomentdatapack.IMGArch.buffer+offset, tomentdatapack.IMGArch.toc[IMG_ID_PLAYER_TANK_CHARACTER].size);
    temp1 = SDL_LoadBMP_RW(sdlWops, SDL_TRUE);
    if(D_CheckTextureLoaded(temp1, IMG_ID_PLAYER_TANK_CHARACTER))
    {
        tomentdatapack.sprites[DS_PlayerTank]->texture = SDL_ConvertSurface(temp1, win_surface->format, SDL_TEXTUREACCESS_TARGET);

        // Load animations as well
        tomentdatapack.sprites[DS_PlayerTank]->animations = (objectanimations_t*)malloc(sizeof(objectanimations_t));
        D_InitializeObjectAnim(tomentdatapack.sprites[DS_PlayerTank]->animations);
        tomentdatapack.sprites[DS_PlayerTank]->animations->belongsTo = tomentdatapack.sprites[DS_PlayerTank];

        // Idle = Normal
        tomentdatapack.sprites[DS_PlayerTank]->animations->animIdle = SDL_ConvertSurface(temp1, win_surface->format, SDL_TEXTUREACCESS_TARGET);
        tomentdatapack.sprites[DS_PlayerTank]->animations->animIdleSheetLength = 0;

        /*
        // Skeleton Death
        int animOffset = tomentdatapack.IMGArch.tocOffset + (tomentdatapack.IMGArch.toc[IMG_ID_SKELETON_LORD_DEATH].startingOffset);
        SDL_RWops* animSdlWops = SDL_RWFromConstMem((byte*)tomentdatapack.IMGArch.buffer+animOffset, tomentdatapack.IMGArch.toc[IMG_ID_SKELETON_LORD_DEATH].size);
        SDL_Surface* animTemp1 = SDL_LoadBMP_RW(animSdlWops, SDL_TRUE);
        tomentdatapack.sprites[DS_SkeletonLord]->animations->animDie = SDL_ConvertSurface(animTemp1, win_surface->format, SDL_TEXTUREACCESS_TARGET);
        tomentdatapack.sprites[DS_SkeletonLord]->animations->animDieSheetLength = 4;
        SDL_FreeSurface(animTemp1);

        // Skeleton Attack
        animOffset = tomentdatapack.IMGArch.tocOffset + (tomentdatapack.IMGArch.toc[IMG_ID_SKELETON_LORD_ATTACK_MELEE].startingOffset);
        animSdlWops = SDL_RWFromConstMem((byte*)tomentdatapack.IMGArch.buffer+animOffset, tomentdatapack.IMGArch.toc[IMG_ID_SKELETON_LORD_ATTACK_MELEE].size);
        animTemp1 = SDL_LoadBMP_RW(animSdlWops, SDL_TRUE);
        tomentdatapack.sprites[DS_SkeletonLord]->animations->animAttack = SDL_ConvertSurface(animTemp1, win_surface->format, SDL_TEXTUREACCESS_TARGET);
        tomentdatapack.sprites[DS_SkeletonLord]->animations->animAttackSheetLength = 4;
        SDL_FreeSurface(animTemp1);

        // Skeleton Cast spell
        animOffset = tomentdatapack.IMGArch.tocOffset + (tomentdatapack.IMGArch.toc[IMG_ID_SKELETON_LORD_ATTACK_FIREBALL].startingOffset);
        animSdlWops = SDL_RWFromConstMem((byte*)tomentdatapack.IMGArch.buffer+animOffset, tomentdatapack.IMGArch.toc[IMG_ID_SKELETON_LORD_ATTACK_FIREBALL].size);
        animTemp1 = SDL_LoadBMP_RW(animSdlWops, SDL_TRUE);
        tomentdatapack.sprites[DS_SkeletonLord]->animations->animCastSpell = SDL_ConvertSurface(animTemp1, win_surface->format, SDL_TEXTUREACCESS_TARGET);
        tomentdatapack.sprites[DS_SkeletonLord]->animations->animCastSpellSheetLength = 5;
        SDL_FreeSurface(animTemp1);

        // Skeleton Special: Hell
        animOffset = tomentdatapack.IMGArch.tocOffset + (tomentdatapack.IMGArch.toc[IMG_ID_SKELETON_LORD_SPELL_HELL].startingOffset);
        animSdlWops = SDL_RWFromConstMem((byte*)tomentdatapack.IMGArch.buffer+animOffset, tomentdatapack.IMGArch.toc[IMG_ID_SKELETON_LORD_SPELL_HELL].size);
        animTemp1 = SDL_LoadBMP_RW(animSdlWops, SDL_TRUE);
        tomentdatapack.sprites[DS_SkeletonLord]->animations->animSpecial1 = SDL_ConvertSurface(animTemp1, win_surface->format, SDL_TEXTUREACCESS_TARGET);
        tomentdatapack.sprites[DS_SkeletonLord]->animations->animSpecial1SheetLength = 3;
        SDL_FreeSurface(animTemp1);

        // Skeleton Special: Resurrection
        animOffset = tomentdatapack.IMGArch.tocOffset + (tomentdatapack.IMGArch.toc[IMG_ID_SKELETON_LORD_SPELL_RESURRECTION].startingOffset);
        animSdlWops = SDL_RWFromConstMem((byte*)tomentdatapack.IMGArch.buffer+animOffset, tomentdatapack.IMGArch.toc[IMG_ID_SKELETON_LORD_SPELL_RESURRECTION].size);
        animTemp1 = SDL_LoadBMP_RW(animSdlWops, SDL_TRUE);
        tomentdatapack.sprites[DS_SkeletonLord]->animations->animSpecial2 = SDL_ConvertSurface(animTemp1, win_surface->format, SDL_TEXTUREACCESS_TARGET);
        tomentdatapack.sprites[DS_SkeletonLord]->animations->animSpecial2SheetLength = 3;
        SDL_FreeSurface(animTemp1);
        */

    }
    else
        tomentdatapack.sprites[DS_PlayerTank]->texture = tomentdatapack.enginesDefaults[EDEFAULT_1]->texture;
    U_SetBit(&tomentdatapack.sprites[DS_PlayerTank]->flags, 0); // Set collision bit flag to 1
    U_SetBit(&tomentdatapack.sprites[DS_PlayerTank]->flags, 2); // Set dynamic bit flag to 1
    U_SetBit(&tomentdatapack.sprites[DS_PlayerTank]->flags, 5); // Set 8 angled sprite

    // Callback
    tomentdatapack.sprites[DS_PlayerTank]->Callback = NULL;
    // Sprite-Specific, set the lookup table for the sprite sheets length
    tomentdatapack.spritesSheetsLenghtTable[DS_PlayerTank] = 0;
    SDL_FreeSurface(temp1);


    // PlayerCharacterHealer
    offset = tomentdatapack.IMGArch.tocOffset + (tomentdatapack.IMGArch.toc[IMG_ID_PLAYER_HEALER_CHARACTER].startingOffset);
    sdlWops = SDL_RWFromConstMem((byte*)tomentdatapack.IMGArch.buffer+offset, tomentdatapack.IMGArch.toc[IMG_ID_PLAYER_HEALER_CHARACTER].size);
    temp1 = SDL_LoadBMP_RW(sdlWops, SDL_TRUE);
    if(D_CheckTextureLoaded(temp1, IMG_ID_PLAYER_HEALER_CHARACTER))
    {
        tomentdatapack.sprites[DS_PlayerHealer]->texture = SDL_ConvertSurface(temp1, win_surface->format, SDL_TEXTUREACCESS_TARGET);

        // Load animations as well
        tomentdatapack.sprites[DS_PlayerHealer]->animations = (objectanimations_t*)malloc(sizeof(objectanimations_t));
        D_InitializeObjectAnim(tomentdatapack.sprites[DS_PlayerHealer]->animations);
        tomentdatapack.sprites[DS_PlayerHealer]->animations->belongsTo = tomentdatapack.sprites[DS_PlayerHealer];

        // Idle = Normal
        tomentdatapack.sprites[DS_PlayerHealer]->animations->animIdle = SDL_ConvertSurface(temp1, win_surface->format, SDL_TEXTUREACCESS_TARGET);
        tomentdatapack.sprites[DS_PlayerHealer]->animations->animIdleSheetLength = 0;

        /*
        // Skeleton Death
        int animOffset = tomentdatapack.IMGArch.tocOffset + (tomentdatapack.IMGArch.toc[IMG_ID_SKELETON_LORD_DEATH].startingOffset);
        SDL_RWops* animSdlWops = SDL_RWFromConstMem((byte*)tomentdatapack.IMGArch.buffer+animOffset, tomentdatapack.IMGArch.toc[IMG_ID_SKELETON_LORD_DEATH].size);
        SDL_Surface* animTemp1 = SDL_LoadBMP_RW(animSdlWops, SDL_TRUE);
        tomentdatapack.sprites[DS_SkeletonLord]->animations->animDie = SDL_ConvertSurface(animTemp1, win_surface->format, SDL_TEXTUREACCESS_TARGET);
        tomentdatapack.sprites[DS_SkeletonLord]->animations->animDieSheetLength = 4;
        SDL_FreeSurface(animTemp1);

        // Skeleton Attack
        animOffset = tomentdatapack.IMGArch.tocOffset + (tomentdatapack.IMGArch.toc[IMG_ID_SKELETON_LORD_ATTACK_MELEE].startingOffset);
        animSdlWops = SDL_RWFromConstMem((byte*)tomentdatapack.IMGArch.buffer+animOffset, tomentdatapack.IMGArch.toc[IMG_ID_SKELETON_LORD_ATTACK_MELEE].size);
        animTemp1 = SDL_LoadBMP_RW(animSdlWops, SDL_TRUE);
        tomentdatapack.sprites[DS_SkeletonLord]->animations->animAttack = SDL_ConvertSurface(animTemp1, win_surface->format, SDL_TEXTUREACCESS_TARGET);
        tomentdatapack.sprites[DS_SkeletonLord]->animations->animAttackSheetLength = 4;
        SDL_FreeSurface(animTemp1);

        // Skeleton Cast spell
        animOffset = tomentdatapack.IMGArch.tocOffset + (tomentdatapack.IMGArch.toc[IMG_ID_SKELETON_LORD_ATTACK_FIREBALL].startingOffset);
        animSdlWops = SDL_RWFromConstMem((byte*)tomentdatapack.IMGArch.buffer+animOffset, tomentdatapack.IMGArch.toc[IMG_ID_SKELETON_LORD_ATTACK_FIREBALL].size);
        animTemp1 = SDL_LoadBMP_RW(animSdlWops, SDL_TRUE);
        tomentdatapack.sprites[DS_SkeletonLord]->animations->animCastSpell = SDL_ConvertSurface(animTemp1, win_surface->format, SDL_TEXTUREACCESS_TARGET);
        tomentdatapack.sprites[DS_SkeletonLord]->animations->animCastSpellSheetLength = 5;
        SDL_FreeSurface(animTemp1);

        // Skeleton Special: Hell
        animOffset = tomentdatapack.IMGArch.tocOffset + (tomentdatapack.IMGArch.toc[IMG_ID_SKELETON_LORD_SPELL_HELL].startingOffset);
        animSdlWops = SDL_RWFromConstMem((byte*)tomentdatapack.IMGArch.buffer+animOffset, tomentdatapack.IMGArch.toc[IMG_ID_SKELETON_LORD_SPELL_HELL].size);
        animTemp1 = SDL_LoadBMP_RW(animSdlWops, SDL_TRUE);
        tomentdatapack.sprites[DS_SkeletonLord]->animations->animSpecial1 = SDL_ConvertSurface(animTemp1, win_surface->format, SDL_TEXTUREACCESS_TARGET);
        tomentdatapack.sprites[DS_SkeletonLord]->animations->animSpecial1SheetLength = 3;
        SDL_FreeSurface(animTemp1);

        // Skeleton Special: Resurrection
        animOffset = tomentdatapack.IMGArch.tocOffset + (tomentdatapack.IMGArch.toc[IMG_ID_SKELETON_LORD_SPELL_RESURRECTION].startingOffset);
        animSdlWops = SDL_RWFromConstMem((byte*)tomentdatapack.IMGArch.buffer+animOffset, tomentdatapack.IMGArch.toc[IMG_ID_SKELETON_LORD_SPELL_RESURRECTION].size);
        animTemp1 = SDL_LoadBMP_RW(animSdlWops, SDL_TRUE);
        tomentdatapack.sprites[DS_SkeletonLord]->animations->animSpecial2 = SDL_ConvertSurface(animTemp1, win_surface->format, SDL_TEXTUREACCESS_TARGET);
        tomentdatapack.sprites[DS_SkeletonLord]->animations->animSpecial2SheetLength = 3;
        SDL_FreeSurface(animTemp1);
        */

    }
    else
        tomentdatapack.sprites[DS_PlayerHealer]->texture = tomentdatapack.enginesDefaults[EDEFAULT_1]->texture;
    U_SetBit(&tomentdatapack.sprites[DS_PlayerHealer]->flags, 0); // Set collision bit flag to 1
    U_SetBit(&tomentdatapack.sprites[DS_PlayerHealer]->flags, 2); // Set dynamic bit flag to 1
    U_SetBit(&tomentdatapack.sprites[DS_PlayerHealer]->flags, 5); // Set 8 angled sprite

    // Callback
    tomentdatapack.sprites[DS_PlayerHealer]->Callback = NULL;
    // Sprite-Specific, set the lookup table for the sprite sheets length
    tomentdatapack.spritesSheetsLenghtTable[DS_PlayerHealer] = 0;
    SDL_FreeSurface(temp1);

    // PlayerCharacterDPS
    offset = tomentdatapack.IMGArch.tocOffset + (tomentdatapack.IMGArch.toc[IMG_ID_PLAYER_DPS_CHARACTER].startingOffset);
    sdlWops = SDL_RWFromConstMem((byte*)tomentdatapack.IMGArch.buffer+offset, tomentdatapack.IMGArch.toc[IMG_ID_PLAYER_DPS_CHARACTER].size);
    temp1 = SDL_LoadBMP_RW(sdlWops, SDL_TRUE);
    if(D_CheckTextureLoaded(temp1, IMG_ID_PLAYER_DPS_CHARACTER))
    {
        tomentdatapack.sprites[DS_PlayerDPS]->texture = SDL_ConvertSurface(temp1, win_surface->format, SDL_TEXTUREACCESS_TARGET);

        // Load animations as well
        tomentdatapack.sprites[DS_PlayerDPS]->animations = (objectanimations_t*)malloc(sizeof(objectanimations_t));
        D_InitializeObjectAnim(tomentdatapack.sprites[DS_PlayerDPS]->animations);
        tomentdatapack.sprites[DS_PlayerDPS]->animations->belongsTo = tomentdatapack.sprites[DS_PlayerDPS];

        // Idle = Normal
        tomentdatapack.sprites[DS_PlayerDPS]->animations->animIdle = SDL_ConvertSurface(temp1, win_surface->format, SDL_TEXTUREACCESS_TARGET);
        tomentdatapack.sprites[DS_PlayerDPS]->animations->animIdleSheetLength = 0;

        /*
        // Skeleton Death
        int animOffset = tomentdatapack.IMGArch.tocOffset + (tomentdatapack.IMGArch.toc[IMG_ID_SKELETON_LORD_DEATH].startingOffset);
        SDL_RWops* animSdlWops = SDL_RWFromConstMem((byte*)tomentdatapack.IMGArch.buffer+animOffset, tomentdatapack.IMGArch.toc[IMG_ID_SKELETON_LORD_DEATH].size);
        SDL_Surface* animTemp1 = SDL_LoadBMP_RW(animSdlWops, SDL_TRUE);
        tomentdatapack.sprites[DS_SkeletonLord]->animations->animDie = SDL_ConvertSurface(animTemp1, win_surface->format, SDL_TEXTUREACCESS_TARGET);
        tomentdatapack.sprites[DS_SkeletonLord]->animations->animDieSheetLength = 4;
        SDL_FreeSurface(animTemp1);

        // Skeleton Attack
        animOffset = tomentdatapack.IMGArch.tocOffset + (tomentdatapack.IMGArch.toc[IMG_ID_SKELETON_LORD_ATTACK_MELEE].startingOffset);
        animSdlWops = SDL_RWFromConstMem((byte*)tomentdatapack.IMGArch.buffer+animOffset, tomentdatapack.IMGArch.toc[IMG_ID_SKELETON_LORD_ATTACK_MELEE].size);
        animTemp1 = SDL_LoadBMP_RW(animSdlWops, SDL_TRUE);
        tomentdatapack.sprites[DS_SkeletonLord]->animations->animAttack = SDL_ConvertSurface(animTemp1, win_surface->format, SDL_TEXTUREACCESS_TARGET);
        tomentdatapack.sprites[DS_SkeletonLord]->animations->animAttackSheetLength = 4;
        SDL_FreeSurface(animTemp1);

        // Skeleton Cast spell
        animOffset = tomentdatapack.IMGArch.tocOffset + (tomentdatapack.IMGArch.toc[IMG_ID_SKELETON_LORD_ATTACK_FIREBALL].startingOffset);
        animSdlWops = SDL_RWFromConstMem((byte*)tomentdatapack.IMGArch.buffer+animOffset, tomentdatapack.IMGArch.toc[IMG_ID_SKELETON_LORD_ATTACK_FIREBALL].size);
        animTemp1 = SDL_LoadBMP_RW(animSdlWops, SDL_TRUE);
        tomentdatapack.sprites[DS_SkeletonLord]->animations->animCastSpell = SDL_ConvertSurface(animTemp1, win_surface->format, SDL_TEXTUREACCESS_TARGET);
        tomentdatapack.sprites[DS_SkeletonLord]->animations->animCastSpellSheetLength = 5;
        SDL_FreeSurface(animTemp1);

        // Skeleton Special: Hell
        animOffset = tomentdatapack.IMGArch.tocOffset + (tomentdatapack.IMGArch.toc[IMG_ID_SKELETON_LORD_SPELL_HELL].startingOffset);
        animSdlWops = SDL_RWFromConstMem((byte*)tomentdatapack.IMGArch.buffer+animOffset, tomentdatapack.IMGArch.toc[IMG_ID_SKELETON_LORD_SPELL_HELL].size);
        animTemp1 = SDL_LoadBMP_RW(animSdlWops, SDL_TRUE);
        tomentdatapack.sprites[DS_SkeletonLord]->animations->animSpecial1 = SDL_ConvertSurface(animTemp1, win_surface->format, SDL_TEXTUREACCESS_TARGET);
        tomentdatapack.sprites[DS_SkeletonLord]->animations->animSpecial1SheetLength = 3;
        SDL_FreeSurface(animTemp1);

        // Skeleton Special: Resurrection
        animOffset = tomentdatapack.IMGArch.tocOffset + (tomentdatapack.IMGArch.toc[IMG_ID_SKELETON_LORD_SPELL_RESURRECTION].startingOffset);
        animSdlWops = SDL_RWFromConstMem((byte*)tomentdatapack.IMGArch.buffer+animOffset, tomentdatapack.IMGArch.toc[IMG_ID_SKELETON_LORD_SPELL_RESURRECTION].size);
        animTemp1 = SDL_LoadBMP_RW(animSdlWops, SDL_TRUE);
        tomentdatapack.sprites[DS_SkeletonLord]->animations->animSpecial2 = SDL_ConvertSurface(animTemp1, win_surface->format, SDL_TEXTUREACCESS_TARGET);
        tomentdatapack.sprites[DS_SkeletonLord]->animations->animSpecial2SheetLength = 3;
        SDL_FreeSurface(animTemp1);
        */

    }
    else
        tomentdatapack.sprites[DS_PlayerDPS]->texture = tomentdatapack.enginesDefaults[EDEFAULT_1]->texture;
    U_SetBit(&tomentdatapack.sprites[DS_PlayerDPS]->flags, 0); // Set collision bit flag to 1
    U_SetBit(&tomentdatapack.sprites[DS_PlayerDPS]->flags, 2); // Set dynamic bit flag to 1
    U_SetBit(&tomentdatapack.sprites[DS_PlayerDPS]->flags, 5); // Set 8 angled sprite

    // Callback
    tomentdatapack.sprites[DS_PlayerDPS]->Callback = NULL;
    // Sprite-Specific, set the lookup table for the sprite sheets length
    tomentdatapack.spritesSheetsLenghtTable[DS_PlayerDPS] = 0;
    SDL_FreeSurface(temp1);

    // Spell Concentrated Heal
    offset = tomentdatapack.IMGArch.tocOffset + (tomentdatapack.IMGArch.toc[IMG_ID_SPELL_CONCENTRATED_HEAL].startingOffset);
    sdlWops = SDL_RWFromConstMem((byte*)tomentdatapack.IMGArch.buffer+offset, tomentdatapack.IMGArch.toc[IMG_ID_SPELL_CONCENTRATED_HEAL].size);
    temp1 = SDL_LoadBMP_RW(sdlWops, SDL_TRUE);
    if(D_CheckTextureLoaded(temp1, IMG_ID_SPELL_CONCENTRATED_HEAL))
    {
        tomentdatapack.sprites[S_ConcentratedHeal]->texture = SDL_ConvertSurface(temp1, win_surface->format, SDL_TEXTUREACCESS_TARGET);

        // Load animations as well
        tomentdatapack.sprites[S_ConcentratedHeal]->animations = (objectanimations_t*)malloc(sizeof(objectanimations_t));
        D_InitializeObjectAnim(tomentdatapack.sprites[S_ConcentratedHeal]->animations);
        tomentdatapack.sprites[S_ConcentratedHeal]->animations->belongsTo = tomentdatapack.sprites[S_ConcentratedHeal];

        // Idle = Normal
        tomentdatapack.sprites[S_ConcentratedHeal]->animations->animIdle = SDL_ConvertSurface(temp1, win_surface->format, SDL_TEXTUREACCESS_TARGET);
        tomentdatapack.sprites[S_ConcentratedHeal]->animations->animIdleSheetLength = 6;

        // Skeleton Death
        int animOffset = tomentdatapack.IMGArch.tocOffset + (tomentdatapack.IMGArch.toc[IMG_ID_SPELL_CONCENTRATED_HEAL_EXPLOSION].startingOffset);
        SDL_RWops* animSdlWops = SDL_RWFromConstMem((byte*)tomentdatapack.IMGArch.buffer+animOffset, tomentdatapack.IMGArch.toc[IMG_ID_SPELL_CONCENTRATED_HEAL_EXPLOSION].size);
        SDL_Surface* animTemp1 = SDL_LoadBMP_RW(animSdlWops, SDL_TRUE);
        tomentdatapack.sprites[S_ConcentratedHeal]->animations->animDie = SDL_ConvertSurface(animTemp1, win_surface->format, SDL_TEXTUREACCESS_TARGET);
        tomentdatapack.sprites[S_ConcentratedHeal]->animations->animDieSheetLength = 5;

        SDL_FreeSurface(animTemp1);
    }
    else
        tomentdatapack.sprites[S_ConcentratedHeal]->texture = tomentdatapack.enginesDefaults[EDEFAULT_1]->texture;
    U_SetBit(&tomentdatapack.sprites[S_ConcentratedHeal]->flags, 0); // Set collision bit flag to 1
    U_SetBit(&tomentdatapack.sprites[S_ConcentratedHeal]->flags, 1); // Set animated sprite bit flag to 1
    // Sprite-Specific, set the lookup table for the sprite sheets length
    tomentdatapack.spritesSheetsLenghtTable[S_ConcentratedHeal] = 6;

    // Callback
    tomentdatapack.sprites[S_ConcentratedHeal]->Callback = NULL;
    SDL_FreeSurface(temp1);

    // Spell Concentrated Heal
    offset = tomentdatapack.IMGArch.tocOffset + (tomentdatapack.IMGArch.toc[IMG_ID_SWORD_PROJECTILE].startingOffset);
    sdlWops = SDL_RWFromConstMem((byte*)tomentdatapack.IMGArch.buffer+offset, tomentdatapack.IMGArch.toc[IMG_ID_SWORD_PROJECTILE].size);
    temp1 = SDL_LoadBMP_RW(sdlWops, SDL_TRUE);
    if(D_CheckTextureLoaded(temp1, IMG_ID_SWORD_PROJECTILE))
    {
        tomentdatapack.sprites[S_SwordProjectile]->texture = SDL_ConvertSurface(temp1, win_surface->format, SDL_TEXTUREACCESS_TARGET);

        // Load animations as well
        tomentdatapack.sprites[S_SwordProjectile]->animations = (objectanimations_t*)malloc(sizeof(objectanimations_t));
        D_InitializeObjectAnim(tomentdatapack.sprites[S_SwordProjectile]->animations);
        tomentdatapack.sprites[S_SwordProjectile]->animations->belongsTo = tomentdatapack.sprites[S_SwordProjectile];

        // Idle = Normal
        tomentdatapack.sprites[S_SwordProjectile]->animations->animIdle = SDL_ConvertSurface(temp1, win_surface->format, SDL_TEXTUREACCESS_TARGET);
        tomentdatapack.sprites[S_SwordProjectile]->animations->animIdleSheetLength = 6;

        // Skeleton Death
        int animOffset = tomentdatapack.IMGArch.tocOffset + (tomentdatapack.IMGArch.toc[IMG_ID_SWORD_PROJECTILE_EXPLOSION].startingOffset);
        SDL_RWops* animSdlWops = SDL_RWFromConstMem((byte*)tomentdatapack.IMGArch.buffer+animOffset, tomentdatapack.IMGArch.toc[IMG_ID_SWORD_PROJECTILE_EXPLOSION].size);
        SDL_Surface* animTemp1 = SDL_LoadBMP_RW(animSdlWops, SDL_TRUE);
        tomentdatapack.sprites[S_SwordProjectile]->animations->animDie = SDL_ConvertSurface(animTemp1, win_surface->format, SDL_TEXTUREACCESS_TARGET);
        tomentdatapack.sprites[S_SwordProjectile]->animations->animDieSheetLength = 5;

        SDL_FreeSurface(animTemp1);
    }
    else
        tomentdatapack.sprites[S_SwordProjectile]->texture = tomentdatapack.enginesDefaults[EDEFAULT_1]->texture;
    U_SetBit(&tomentdatapack.sprites[S_SwordProjectile]->flags, 0); // Set collision bit flag to 1
    U_SetBit(&tomentdatapack.sprites[S_SwordProjectile]->flags, 1); // Set animated sprite bit flag to 1
    // Sprite-Specific, set the lookup table for the sprite sheets length
    tomentdatapack.spritesSheetsLenghtTable[S_SwordProjectile] = 6;

    // Callback
    tomentdatapack.sprites[S_SwordProjectile]->Callback = NULL;
    SDL_FreeSurface(temp1);

     // LOAD DYNAMIC
    // AI DS_MorgathulTheKeeper
    offset = tomentdatapack.IMGArch.tocOffset + (tomentdatapack.IMGArch.toc[IMG_ID_MORGATHUL_IDLE].startingOffset);
    sdlWops = SDL_RWFromConstMem((byte*)tomentdatapack.IMGArch.buffer+offset, tomentdatapack.IMGArch.toc[IMG_ID_MORGATHUL_IDLE].size);
    temp1 = SDL_LoadBMP_RW(sdlWops, SDL_TRUE);
    if(D_CheckTextureLoaded(temp1, IMG_ID_MORGATHUL_IDLE))
    {
        tomentdatapack.sprites[DS_MorgathulTheKeeper]->texture = SDL_ConvertSurface(temp1, win_surface->format, SDL_TEXTUREACCESS_TARGET);

        // Load animations as well
        tomentdatapack.sprites[DS_MorgathulTheKeeper]->animations = (objectanimations_t*)malloc(sizeof(objectanimations_t));
        D_InitializeObjectAnim(tomentdatapack.sprites[DS_MorgathulTheKeeper]->animations);
        tomentdatapack.sprites[DS_MorgathulTheKeeper]->animations->belongsTo = tomentdatapack.sprites[DS_MorgathulTheKeeper];

        // Idle = Normal
        tomentdatapack.sprites[DS_MorgathulTheKeeper]->animations->animIdle = SDL_ConvertSurface(temp1, win_surface->format, SDL_TEXTUREACCESS_TARGET);
        tomentdatapack.sprites[DS_MorgathulTheKeeper]->animations->animIdleSheetLength = 4;

        // Death
        int animOffset = tomentdatapack.IMGArch.tocOffset + (tomentdatapack.IMGArch.toc[IMG_ID_MORGATHUL_DEATH].startingOffset);
        SDL_RWops* animSdlWops = SDL_RWFromConstMem((byte*)tomentdatapack.IMGArch.buffer+animOffset, tomentdatapack.IMGArch.toc[IMG_ID_MORGATHUL_DEATH].size);
        SDL_Surface* animTemp1 = SDL_LoadBMP_RW(animSdlWops, SDL_TRUE);
        tomentdatapack.sprites[DS_MorgathulTheKeeper]->animations->animDie = SDL_ConvertSurface(animTemp1, win_surface->format, SDL_TEXTUREACCESS_TARGET);
        tomentdatapack.sprites[DS_MorgathulTheKeeper]->animations->animDieSheetLength = 7;
        SDL_FreeSurface(animTemp1);

        // Attack melee
        animOffset = tomentdatapack.IMGArch.tocOffset + (tomentdatapack.IMGArch.toc[IMG_ID_MORGATHUL_CAST2].startingOffset);
        animSdlWops = SDL_RWFromConstMem((byte*)tomentdatapack.IMGArch.buffer+animOffset, tomentdatapack.IMGArch.toc[IMG_ID_MORGATHUL_CAST2].size);
        animTemp1 = SDL_LoadBMP_RW(animSdlWops, SDL_TRUE);
        tomentdatapack.sprites[DS_MorgathulTheKeeper]->animations->animAttack = SDL_ConvertSurface(animTemp1, win_surface->format, SDL_TEXTUREACCESS_TARGET);
        tomentdatapack.sprites[DS_MorgathulTheKeeper]->animations->animAttackSheetLength = 4;
        SDL_FreeSurface(animTemp1);

        // Cast spell
        animOffset = tomentdatapack.IMGArch.tocOffset + (tomentdatapack.IMGArch.toc[IMG_ID_MORGATHUL_CAST2].startingOffset);
        animSdlWops = SDL_RWFromConstMem((byte*)tomentdatapack.IMGArch.buffer+animOffset, tomentdatapack.IMGArch.toc[IMG_ID_MORGATHUL_CAST2].size);
        animTemp1 = SDL_LoadBMP_RW(animSdlWops, SDL_TRUE);
        tomentdatapack.sprites[DS_MorgathulTheKeeper]->animations->animCastSpell = SDL_ConvertSurface(animTemp1, win_surface->format, SDL_TEXTUREACCESS_TARGET);
        tomentdatapack.sprites[DS_MorgathulTheKeeper]->animations->animCastSpellSheetLength = 4;
        SDL_FreeSurface(animTemp1);

        // Special: Resurrect K
        animOffset = tomentdatapack.IMGArch.tocOffset + (tomentdatapack.IMGArch.toc[IMG_ID_MORGATHUL_CAST1].startingOffset);
        animSdlWops = SDL_RWFromConstMem((byte*)tomentdatapack.IMGArch.buffer+animOffset, tomentdatapack.IMGArch.toc[IMG_ID_MORGATHUL_CAST1].size);
        animTemp1 = SDL_LoadBMP_RW(animSdlWops, SDL_TRUE);
        tomentdatapack.sprites[DS_MorgathulTheKeeper]->animations->animSpecial1 = SDL_ConvertSurface(animTemp1, win_surface->format, SDL_TEXTUREACCESS_TARGET);
        tomentdatapack.sprites[DS_MorgathulTheKeeper]->animations->animSpecial1SheetLength = 7;
        tomentdatapack.sprites[DS_MorgathulTheKeeper]->animations->animSpecial1ActionFrame = 4;
        
        SDL_FreeSurface(animTemp1);

        // Violet void/copy
        animOffset = tomentdatapack.IMGArch.tocOffset + (tomentdatapack.IMGArch.toc[IMG_ID_MORGATHUL_CAST1].startingOffset);
        animSdlWops = SDL_RWFromConstMem((byte*)tomentdatapack.IMGArch.buffer+animOffset, tomentdatapack.IMGArch.toc[IMG_ID_MORGATHUL_CAST1].size);
        animTemp1 = SDL_LoadBMP_RW(animSdlWops, SDL_TRUE);
        tomentdatapack.sprites[DS_MorgathulTheKeeper]->animations->animSpecial2 = SDL_ConvertSurface(animTemp1, win_surface->format, SDL_TEXTUREACCESS_TARGET);
        tomentdatapack.sprites[DS_MorgathulTheKeeper]->animations->animSpecial2SheetLength = 7;
        tomentdatapack.sprites[DS_MorgathulTheKeeper]->animations->animSpecial2ActionFrame = 4;
        
        SDL_FreeSurface(animTemp1);

        // Orb storm
        animOffset = tomentdatapack.IMGArch.tocOffset + (tomentdatapack.IMGArch.toc[IMG_ID_MORGATHUL_CAST2].startingOffset);
        animSdlWops = SDL_RWFromConstMem((byte*)tomentdatapack.IMGArch.buffer+animOffset, tomentdatapack.IMGArch.toc[IMG_ID_MORGATHUL_CAST2].size);
        animTemp1 = SDL_LoadBMP_RW(animSdlWops, SDL_TRUE);
        tomentdatapack.sprites[DS_MorgathulTheKeeper]->animations->animSpecial3 = SDL_ConvertSurface(animTemp1, win_surface->format, SDL_TEXTUREACCESS_TARGET);
        tomentdatapack.sprites[DS_MorgathulTheKeeper]->animations->animSpecial3SheetLength = 4;
        tomentdatapack.sprites[DS_MorgathulTheKeeper]->animations->animSpecial3ActionFrame = 3;
        
        SDL_FreeSurface(animTemp1);
    }
    else
        tomentdatapack.sprites[DS_MorgathulTheKeeper]->texture = tomentdatapack.enginesDefaults[EDEFAULT_1]->texture;
    U_SetBit(&tomentdatapack.sprites[DS_MorgathulTheKeeper]->flags, 0); // Set collision bit flag to 1
    U_SetBit(&tomentdatapack.sprites[DS_MorgathulTheKeeper]->flags, 2); // Set dynamic bit flag to 1

    // Callback
    tomentdatapack.sprites[DS_MorgathulTheKeeper]->Callback = NULL;
    SDL_FreeSurface(temp1);

    // Spell Morgathul Orb
    offset = tomentdatapack.IMGArch.tocOffset + (tomentdatapack.IMGArch.toc[IMG_ID_SPELL_MORGATHUL_ORB].startingOffset);
    sdlWops = SDL_RWFromConstMem((byte*)tomentdatapack.IMGArch.buffer+offset, tomentdatapack.IMGArch.toc[IMG_ID_SPELL_MORGATHUL_ORB].size);
    temp1 = SDL_LoadBMP_RW(sdlWops, SDL_TRUE);
    if(D_CheckTextureLoaded(temp1, IMG_ID_SPELL_MORGATHUL_ORB))
    {
        tomentdatapack.sprites[S_MorgathulOrb]->texture = SDL_ConvertSurface(temp1, win_surface->format, SDL_TEXTUREACCESS_TARGET);

        // Load animations as well
        tomentdatapack.sprites[S_MorgathulOrb]->animations = (objectanimations_t*)malloc(sizeof(objectanimations_t));
        D_InitializeObjectAnim(tomentdatapack.sprites[S_MorgathulOrb]->animations);
        tomentdatapack.sprites[S_MorgathulOrb]->animations->belongsTo = tomentdatapack.sprites[S_MorgathulOrb];

        // Idle = Normal
        tomentdatapack.sprites[S_MorgathulOrb]->animations->animIdle = SDL_ConvertSurface(temp1, win_surface->format, SDL_TEXTUREACCESS_TARGET);
        tomentdatapack.sprites[S_MorgathulOrb]->animations->animIdleSheetLength = 4;

        // Skeleton Death
        int animOffset = tomentdatapack.IMGArch.tocOffset + (tomentdatapack.IMGArch.toc[IMG_ID_SPELL_MORGATHUL_ORB_EXPLOSION].startingOffset);
        SDL_RWops* animSdlWops = SDL_RWFromConstMem((byte*)tomentdatapack.IMGArch.buffer+animOffset, tomentdatapack.IMGArch.toc[IMG_ID_SPELL_MORGATHUL_ORB_EXPLOSION].size);
        SDL_Surface* animTemp1 = SDL_LoadBMP_RW(animSdlWops, SDL_TRUE);
        tomentdatapack.sprites[S_MorgathulOrb]->animations->animDie = SDL_ConvertSurface(animTemp1, win_surface->format, SDL_TEXTUREACCESS_TARGET);
        tomentdatapack.sprites[S_MorgathulOrb]->animations->animDieSheetLength = 4;

        SDL_FreeSurface(animTemp1);
    }
    else
        tomentdatapack.sprites[S_MorgathulOrb]->texture = tomentdatapack.enginesDefaults[EDEFAULT_1]->texture;
    U_SetBit(&tomentdatapack.sprites[S_MorgathulOrb]->flags, 0); // Set collision bit flag to 1
    U_SetBit(&tomentdatapack.sprites[S_MorgathulOrb]->flags, 1); // Set animated sprite bit flag to 1
    // Sprite-Specific, set the lookup table for the sprite sheets length
    tomentdatapack.spritesSheetsLenghtTable[S_MorgathulOrb] = 4;

    // Callback
    tomentdatapack.sprites[S_MorgathulOrb]->Callback = NULL;
    SDL_FreeSurface(temp1);

    // AI Kroganar
    offset = tomentdatapack.IMGArch.tocOffset + (tomentdatapack.IMGArch.toc[IMG_ID_KROGANAR_IDLE].startingOffset);
    sdlWops = SDL_RWFromConstMem((byte*)tomentdatapack.IMGArch.buffer+offset, tomentdatapack.IMGArch.toc[IMG_ID_KROGANAR_IDLE].size);
    temp1 = SDL_LoadBMP_RW(sdlWops, SDL_TRUE);
    if(D_CheckTextureLoaded(temp1, IMG_ID_KROGANAR_IDLE))
    {
        tomentdatapack.sprites[DS_Kroganar]->texture = SDL_ConvertSurface(temp1, win_surface->format, SDL_TEXTUREACCESS_TARGET);

        // Load animations as well
        tomentdatapack.sprites[DS_Kroganar]->animations = (objectanimations_t*)malloc(sizeof(objectanimations_t));
        D_InitializeObjectAnim(tomentdatapack.sprites[DS_Kroganar]->animations);
        tomentdatapack.sprites[DS_Kroganar]->animations->belongsTo = tomentdatapack.sprites[DS_Kroganar];

        // Idle = Normal
        tomentdatapack.sprites[DS_Kroganar]->animations->animIdle = SDL_ConvertSurface(temp1, win_surface->format, SDL_TEXTUREACCESS_TARGET);
        tomentdatapack.sprites[DS_Kroganar]->animations->animIdleSheetLength = 0;

        // Skeleton Death
        int animOffset = tomentdatapack.IMGArch.tocOffset + (tomentdatapack.IMGArch.toc[IMG_ID_KROGANAR_DEATH].startingOffset);
        SDL_RWops* animSdlWops = SDL_RWFromConstMem((byte*)tomentdatapack.IMGArch.buffer+animOffset, tomentdatapack.IMGArch.toc[IMG_ID_KROGANAR_DEATH].size);
        SDL_Surface* animTemp1 = SDL_LoadBMP_RW(animSdlWops, SDL_TRUE);
        tomentdatapack.sprites[DS_Kroganar]->animations->animDie = SDL_ConvertSurface(animTemp1, win_surface->format, SDL_TEXTUREACCESS_TARGET);
        tomentdatapack.sprites[DS_Kroganar]->animations->animDieSheetLength = 4;
        SDL_FreeSurface(animTemp1);

        // Skeleton Attack
        animOffset = tomentdatapack.IMGArch.tocOffset + (tomentdatapack.IMGArch.toc[IMG_ID_KROGANAR_ATTACK].startingOffset);
        animSdlWops = SDL_RWFromConstMem((byte*)tomentdatapack.IMGArch.buffer+animOffset, tomentdatapack.IMGArch.toc[IMG_ID_KROGANAR_ATTACK].size);
        animTemp1 = SDL_LoadBMP_RW(animSdlWops, SDL_TRUE);
        tomentdatapack.sprites[DS_Kroganar]->animations->animAttack = SDL_ConvertSurface(animTemp1, win_surface->format, SDL_TEXTUREACCESS_TARGET);
        tomentdatapack.sprites[DS_Kroganar]->animations->animAttackSheetLength = 3;
        SDL_FreeSurface(animTemp1);

        // Skeleton Resurrect
        animOffset = tomentdatapack.IMGArch.tocOffset + (tomentdatapack.IMGArch.toc[IMG_ID_KROGANAR_RESURRECTION].startingOffset);
        animSdlWops = SDL_RWFromConstMem((byte*)tomentdatapack.IMGArch.buffer+animOffset, tomentdatapack.IMGArch.toc[IMG_ID_KROGANAR_RESURRECTION].size);
        animTemp1 = SDL_LoadBMP_RW(animSdlWops, SDL_TRUE);
        tomentdatapack.sprites[DS_Kroganar]->animations->animSpecial1 = SDL_ConvertSurface(animTemp1, win_surface->format, SDL_TEXTUREACCESS_TARGET);
        tomentdatapack.sprites[DS_Kroganar]->animations->animSpecial1SheetLength = 4;
        SDL_FreeSurface(animTemp1);
    }
    else
        tomentdatapack.sprites[DS_Kroganar]->texture = tomentdatapack.enginesDefaults[EDEFAULT_1]->texture;
    U_SetBit(&tomentdatapack.sprites[DS_Kroganar]->flags, 0); // Set collision bit flag to 1
    U_SetBit(&tomentdatapack.sprites[DS_Kroganar]->flags, 2); // Set dynamic bit flag to 1

    // Callback
    tomentdatapack.sprites[DS_Kroganar]->Callback = NULL;
    SDL_FreeSurface(temp1);

    // AI DS_MorgathulCopy
    offset = tomentdatapack.IMGArch.tocOffset + (tomentdatapack.IMGArch.toc[IMG_ID_MORGATHUL_IDLE].startingOffset);
    sdlWops = SDL_RWFromConstMem((byte*)tomentdatapack.IMGArch.buffer+offset, tomentdatapack.IMGArch.toc[IMG_ID_MORGATHUL_IDLE].size);
    temp1 = SDL_LoadBMP_RW(sdlWops, SDL_TRUE);
    if(D_CheckTextureLoaded(temp1, IMG_ID_MORGATHUL_IDLE))
    {
        tomentdatapack.sprites[DS_MorgathulCopy]->texture = SDL_ConvertSurface(temp1, win_surface->format, SDL_TEXTUREACCESS_TARGET);

        // Load animations as well
        tomentdatapack.sprites[DS_MorgathulCopy]->animations = (objectanimations_t*)malloc(sizeof(objectanimations_t));
        D_InitializeObjectAnim(tomentdatapack.sprites[DS_MorgathulCopy]->animations);
        tomentdatapack.sprites[DS_MorgathulCopy]->animations->belongsTo = tomentdatapack.sprites[DS_MorgathulCopy];

        // Idle = Normal
        tomentdatapack.sprites[DS_MorgathulCopy]->animations->animIdle = SDL_ConvertSurface(temp1, win_surface->format, SDL_TEXTUREACCESS_TARGET);
        tomentdatapack.sprites[DS_MorgathulCopy]->animations->animIdleSheetLength = 4;

        // Death
        int animOffset = tomentdatapack.IMGArch.tocOffset + (tomentdatapack.IMGArch.toc[IMG_ID_MORGATHUL_COPY_DEATH].startingOffset);
        SDL_RWops* animSdlWops = SDL_RWFromConstMem((byte*)tomentdatapack.IMGArch.buffer+animOffset, tomentdatapack.IMGArch.toc[IMG_ID_MORGATHUL_COPY_DEATH].size);
        SDL_Surface* animTemp1 = SDL_LoadBMP_RW(animSdlWops, SDL_TRUE);
        tomentdatapack.sprites[DS_MorgathulCopy]->animations->animDie = SDL_ConvertSurface(animTemp1, win_surface->format, SDL_TEXTUREACCESS_TARGET);
        tomentdatapack.sprites[DS_MorgathulCopy]->animations->animDieSheetLength = 5;
        SDL_FreeSurface(animTemp1);

        // Attack melee
        animOffset = tomentdatapack.IMGArch.tocOffset + (tomentdatapack.IMGArch.toc[IMG_ID_MORGATHUL_CAST2].startingOffset);
        animSdlWops = SDL_RWFromConstMem((byte*)tomentdatapack.IMGArch.buffer+animOffset, tomentdatapack.IMGArch.toc[IMG_ID_MORGATHUL_CAST2].size);
        animTemp1 = SDL_LoadBMP_RW(animSdlWops, SDL_TRUE);
        tomentdatapack.sprites[DS_MorgathulCopy]->animations->animAttack = SDL_ConvertSurface(animTemp1, win_surface->format, SDL_TEXTUREACCESS_TARGET);
        tomentdatapack.sprites[DS_MorgathulCopy]->animations->animAttackSheetLength = 4;
        SDL_FreeSurface(animTemp1);

        // Cast spell
        animOffset = tomentdatapack.IMGArch.tocOffset + (tomentdatapack.IMGArch.toc[IMG_ID_MORGATHUL_CAST2].startingOffset);
        animSdlWops = SDL_RWFromConstMem((byte*)tomentdatapack.IMGArch.buffer+animOffset, tomentdatapack.IMGArch.toc[IMG_ID_MORGATHUL_CAST2].size);
        animTemp1 = SDL_LoadBMP_RW(animSdlWops, SDL_TRUE);
        tomentdatapack.sprites[DS_MorgathulCopy]->animations->animCastSpell = SDL_ConvertSurface(animTemp1, win_surface->format, SDL_TEXTUREACCESS_TARGET);
        tomentdatapack.sprites[DS_MorgathulCopy]->animations->animCastSpellSheetLength = 4;
        SDL_FreeSurface(animTemp1);

        // Resurrect
        animOffset = tomentdatapack.IMGArch.tocOffset + (tomentdatapack.IMGArch.toc[IMG_ID_MORGATHUL_COPY_RESURRECTION].startingOffset);
        animSdlWops = SDL_RWFromConstMem((byte*)tomentdatapack.IMGArch.buffer+animOffset, tomentdatapack.IMGArch.toc[IMG_ID_MORGATHUL_COPY_RESURRECTION].size);
        animTemp1 = SDL_LoadBMP_RW(animSdlWops, SDL_TRUE);
        tomentdatapack.sprites[DS_MorgathulCopy]->animations->animSpecial1 = SDL_ConvertSurface(animTemp1, win_surface->format, SDL_TEXTUREACCESS_TARGET);
        tomentdatapack.sprites[DS_MorgathulCopy]->animations->animSpecial1SheetLength = 4;
        SDL_FreeSurface(animTemp1);
    }
    else
        tomentdatapack.sprites[DS_MorgathulCopy]->texture = tomentdatapack.enginesDefaults[EDEFAULT_1]->texture;
    U_SetBit(&tomentdatapack.sprites[DS_MorgathulCopy]->flags, 0); // Set collision bit flag to 1
    U_SetBit(&tomentdatapack.sprites[DS_MorgathulCopy]->flags, 2); // Set dynamic bit flag to 1

    // Callback
    tomentdatapack.sprites[DS_MorgathulCopy]->Callback = NULL;
    SDL_FreeSurface(temp1);

    // Altar Health
    offset = tomentdatapack.IMGArch.tocOffset + (tomentdatapack.IMGArch.toc[IMG_ID_S_ALTAR_SPELL_POWER].startingOffset);
    sdlWops = SDL_RWFromConstMem((byte*)tomentdatapack.IMGArch.buffer+offset, tomentdatapack.IMGArch.toc[IMG_ID_S_ALTAR_SPELL_POWER].size);
    temp1 = SDL_LoadBMP_RW(sdlWops, SDL_TRUE);
    if(D_CheckTextureLoaded(temp1, IMG_ID_S_ALTAR_SPELL_POWER))
        tomentdatapack.sprites[S_AltarSpellPower]->texture = SDL_ConvertSurface(temp1, win_surface->format, SDL_TEXTUREACCESS_TARGET);
    else
        tomentdatapack.sprites[S_AltarSpellPower]->texture = tomentdatapack.enginesDefaults[EDEFAULT_1]->texture;
    U_SetBit(&tomentdatapack.sprites[S_AltarSpellPower]->flags, 0); // Set collision bit flag to 1
    // Sprite-Specific, set the lookup table for the sprite sheets length
    tomentdatapack.spritesSheetsLenghtTable[S_AltarSpellPower] = 0;

    // Callback
    tomentdatapack.sprites[S_AltarSpellPower]->Callback = D_CallbackUseAltar;
    tomentdatapack.sprites[S_AltarSpellPower]->data = "SPELLPOWER";
    SDL_FreeSurface(temp1);

     // AI DS_TheFrozenLord
    offset = tomentdatapack.IMGArch.tocOffset + (tomentdatapack.IMGArch.toc[IMG_ID_S_THEFROZENLORD_IDLE].startingOffset);
    sdlWops = SDL_RWFromConstMem((byte*)tomentdatapack.IMGArch.buffer+offset, tomentdatapack.IMGArch.toc[IMG_ID_S_THEFROZENLORD_IDLE].size);
    temp1 = SDL_LoadBMP_RW(sdlWops, SDL_TRUE);
    if(D_CheckTextureLoaded(temp1, IMG_ID_S_THEFROZENLORD_IDLE))
    {
        tomentdatapack.sprites[DS_TheFrozenLord]->texture = SDL_ConvertSurface(temp1, win_surface->format, SDL_TEXTUREACCESS_TARGET);

        // Load animations as well
        tomentdatapack.sprites[DS_TheFrozenLord]->animations = (objectanimations_t*)malloc(sizeof(objectanimations_t));
        D_InitializeObjectAnim(tomentdatapack.sprites[DS_TheFrozenLord]->animations);
        tomentdatapack.sprites[DS_TheFrozenLord]->animations->belongsTo = tomentdatapack.sprites[DS_TheFrozenLord];

        // Idle = Normal
        tomentdatapack.sprites[DS_TheFrozenLord]->animations->animIdle = SDL_ConvertSurface(temp1, win_surface->format, SDL_TEXTUREACCESS_TARGET);
        tomentdatapack.sprites[DS_TheFrozenLord]->animations->animIdleSheetLength = 10;
        tomentdatapack.sprites[DS_TheFrozenLord]->animations->animIdleSpeedModifier = -100;

        // Death
        int animOffset = tomentdatapack.IMGArch.tocOffset + (tomentdatapack.IMGArch.toc[IMG_ID_S_THEFROZENLORD_DEATH].startingOffset);
        SDL_RWops* animSdlWops = SDL_RWFromConstMem((byte*)tomentdatapack.IMGArch.buffer+animOffset, tomentdatapack.IMGArch.toc[IMG_ID_S_THEFROZENLORD_DEATH].size);
        SDL_Surface* animTemp1 = SDL_LoadBMP_RW(animSdlWops, SDL_TRUE);
        tomentdatapack.sprites[DS_TheFrozenLord]->animations->animDie = SDL_ConvertSurface(animTemp1, win_surface->format, SDL_TEXTUREACCESS_TARGET);
        tomentdatapack.sprites[DS_TheFrozenLord]->animations->animDieSheetLength = 11;
        tomentdatapack.sprites[DS_TheFrozenLord]->animations->animDieSpeedModifier = -100;
        SDL_FreeSurface(animTemp1);

        // Attack melee
        animOffset = tomentdatapack.IMGArch.tocOffset + (tomentdatapack.IMGArch.toc[IMG_ID_S_THEFROZENLORD_ATTACK1].startingOffset);
        animSdlWops = SDL_RWFromConstMem((byte*)tomentdatapack.IMGArch.buffer+animOffset, tomentdatapack.IMGArch.toc[IMG_ID_S_THEFROZENLORD_ATTACK1].size);
        animTemp1 = SDL_LoadBMP_RW(animSdlWops, SDL_TRUE);
        tomentdatapack.sprites[DS_TheFrozenLord]->animations->animAttack = SDL_ConvertSurface(animTemp1, win_surface->format, SDL_TEXTUREACCESS_TARGET);
        tomentdatapack.sprites[DS_TheFrozenLord]->animations->animAttackSheetLength = 10;
        tomentdatapack.sprites[DS_TheFrozenLord]->animations->animAttackActionFrame = 5;
        tomentdatapack.sprites[DS_TheFrozenLord]->animations->animAttackSpeedModifier = -100;
        SDL_FreeSurface(animTemp1);

        // Cast
        animOffset = tomentdatapack.IMGArch.tocOffset + (tomentdatapack.IMGArch.toc[IMG_ID_S_THEFROZENLORD_ATTACK2].startingOffset);
        animSdlWops = SDL_RWFromConstMem((byte*)tomentdatapack.IMGArch.buffer+animOffset, tomentdatapack.IMGArch.toc[IMG_ID_S_THEFROZENLORD_ATTACK2].size);
        animTemp1 = SDL_LoadBMP_RW(animSdlWops, SDL_TRUE);
        tomentdatapack.sprites[DS_TheFrozenLord]->animations->animCastSpell = SDL_ConvertSurface(animTemp1, win_surface->format, SDL_TEXTUREACCESS_TARGET);
        tomentdatapack.sprites[DS_TheFrozenLord]->animations->animCastSpellSheetLength = 9;
        tomentdatapack.sprites[DS_TheFrozenLord]->animations->animCastSpellkSpeedModifier = -100;
        tomentdatapack.sprites[DS_TheFrozenLord]->animations->animCastSpellActionFrame = 4;
        SDL_FreeSurface(animTemp1);

        // Blizzard
        animOffset = tomentdatapack.IMGArch.tocOffset + (tomentdatapack.IMGArch.toc[IMG_ID_S_THEFROZENLORD_IDLE].startingOffset);
        animSdlWops = SDL_RWFromConstMem((byte*)tomentdatapack.IMGArch.buffer+animOffset, tomentdatapack.IMGArch.toc[IMG_ID_S_THEFROZENLORD_IDLE].size);
        animTemp1 = SDL_LoadBMP_RW(animSdlWops, SDL_TRUE);
        tomentdatapack.sprites[DS_TheFrozenLord]->animations->animSpecial1 = SDL_ConvertSurface(animTemp1, win_surface->format, SDL_TEXTUREACCESS_TARGET);
        tomentdatapack.sprites[DS_TheFrozenLord]->animations->animSpecial1SheetLength = 10;
        tomentdatapack.sprites[DS_TheFrozenLord]->animations->animSpecial1SpeedModifier = -100;
        
        SDL_FreeSurface(animTemp1);

        // Resurrections
        animOffset = tomentdatapack.IMGArch.tocOffset + (tomentdatapack.IMGArch.toc[IMG_ID_S_THEFROZENLORD_IDLE].startingOffset);
        animSdlWops = SDL_RWFromConstMem((byte*)tomentdatapack.IMGArch.buffer+animOffset, tomentdatapack.IMGArch.toc[IMG_ID_S_THEFROZENLORD_IDLE].size);
        animTemp1 = SDL_LoadBMP_RW(animSdlWops, SDL_TRUE);
        tomentdatapack.sprites[DS_TheFrozenLord]->animations->animSpecial2 = SDL_ConvertSurface(animTemp1, win_surface->format, SDL_TEXTUREACCESS_TARGET);
        tomentdatapack.sprites[DS_TheFrozenLord]->animations->animSpecial2SheetLength = 10;
        tomentdatapack.sprites[DS_TheFrozenLord]->animations->animSpecial2SpeedModifier = -100;

        // Ground strike
        animOffset = tomentdatapack.IMGArch.tocOffset + (tomentdatapack.IMGArch.toc[IMG_ID_S_THEFROZENLORD_ATTACK3].startingOffset);
        animSdlWops = SDL_RWFromConstMem((byte*)tomentdatapack.IMGArch.buffer+animOffset, tomentdatapack.IMGArch.toc[IMG_ID_S_THEFROZENLORD_ATTACK3].size);
        animTemp1 = SDL_LoadBMP_RW(animSdlWops, SDL_TRUE);
        tomentdatapack.sprites[DS_TheFrozenLord]->animations->animSpecial3 = SDL_ConvertSurface(animTemp1, win_surface->format, SDL_TEXTUREACCESS_TARGET);
        tomentdatapack.sprites[DS_TheFrozenLord]->animations->animSpecial3SheetLength = 22;
        tomentdatapack.sprites[DS_TheFrozenLord]->animations->animSpecial3ActionFrame = 14;
        tomentdatapack.sprites[DS_TheFrozenLord]->animations->animSpecial3SpeedModifier = -100;

        SDL_FreeSurface(animTemp1);

        /*
        // Orb storm
        animOffset = tomentdatapack.IMGArch.tocOffset + (tomentdatapack.IMGArch.toc[IMG_ID_MORGATHUL_CAST2].startingOffset);
        animSdlWops = SDL_RWFromConstMem((byte*)tomentdatapack.IMGArch.buffer+animOffset, tomentdatapack.IMGArch.toc[IMG_ID_MORGATHUL_CAST2].size);
        animTemp1 = SDL_LoadBMP_RW(animSdlWops, SDL_TRUE);
        tomentdatapack.sprites[DS_TheFrozenLord]->animations->animSpecial3 = SDL_ConvertSurface(animTemp1, win_surface->format, SDL_TEXTUREACCESS_TARGET);
        tomentdatapack.sprites[DS_TheFrozenLord]->animations->animSpecial3SheetLength = 4;
        tomentdatapack.sprites[DS_TheFrozenLord]->animations->animSpecial3ActionFrame = 3;
        
        SDL_FreeSurface(animTemp1);
        */
    }
    else
        tomentdatapack.sprites[DS_TheFrozenLord]->texture = tomentdatapack.enginesDefaults[EDEFAULT_1]->texture;
    U_SetBit(&tomentdatapack.sprites[DS_TheFrozenLord]->flags, 0); // Set collision bit flag to 1
    U_SetBit(&tomentdatapack.sprites[DS_TheFrozenLord]->flags, 2); // Set dynamic bit flag to 1

    // Callback
    tomentdatapack.sprites[DS_TheFrozenLord]->Callback = NULL;
    SDL_FreeSurface(temp1);

    // Pickup Change skybox
    offset = tomentdatapack.IMGArch.tocOffset + (tomentdatapack.IMGArch.toc[IMG_ID_EMPTY_IMG].startingOffset);
    sdlWops = SDL_RWFromConstMem((byte*)tomentdatapack.IMGArch.buffer+offset, tomentdatapack.IMGArch.toc[IMG_ID_EMPTY_IMG].size);
    temp1 = SDL_LoadBMP_RW(sdlWops, SDL_TRUE);
    if(D_CheckTextureLoaded(temp1, IMG_ID_EMPTY_IMG))
        tomentdatapack.sprites[S_PickupChangeEnvironmentTheFrozenEnd]->texture = SDL_ConvertSurface(temp1, win_surface->format, SDL_TEXTUREACCESS_TARGET);
    else
        tomentdatapack.sprites[S_PickupChangeEnvironmentTheFrozenEnd]->texture = tomentdatapack.enginesDefaults[EDEFAULT_1]->texture;

    U_SetBit(&tomentdatapack.sprites[S_PickupChangeEnvironmentTheFrozenEnd]->flags, 3); // Auto call callback upon player's collision (to pickup the potion by touching it)

    // Callback
    tomentdatapack.sprites[S_PickupChangeEnvironmentTheFrozenEnd]->Callback = D_CallbackTheFrozenEndFinalEncounter;
    tomentdatapack.sprites[S_PickupChangeEnvironmentTheFrozenEnd]->data = "";
    SDL_FreeSurface(temp1);


    // Spell Ice Blast
    offset = tomentdatapack.IMGArch.tocOffset + (tomentdatapack.IMGArch.toc[IMG_ID_SPELL_ICE_BLAST].startingOffset);
    sdlWops = SDL_RWFromConstMem((byte*)tomentdatapack.IMGArch.buffer+offset, tomentdatapack.IMGArch.toc[IMG_ID_SPELL_ICE_BLAST].size);
    temp1 = SDL_LoadBMP_RW(sdlWops, SDL_TRUE);
    if(D_CheckTextureLoaded(temp1, IMG_ID_SPELL_ICE_BLAST))
    {
        tomentdatapack.sprites[S_IceBlast]->texture = SDL_ConvertSurface(temp1, win_surface->format, SDL_TEXTUREACCESS_TARGET);

        // Load animations as well
        tomentdatapack.sprites[S_IceBlast]->animations = (objectanimations_t*)malloc(sizeof(objectanimations_t));
        D_InitializeObjectAnim(tomentdatapack.sprites[S_IceBlast]->animations);
        tomentdatapack.sprites[S_IceBlast]->animations->belongsTo = tomentdatapack.sprites[S_IceBlast];

        // Idle = Normal
        tomentdatapack.sprites[S_IceBlast]->animations->animIdle = SDL_ConvertSurface(temp1, win_surface->format, SDL_TEXTUREACCESS_TARGET);
        tomentdatapack.sprites[S_IceBlast]->animations->animIdleSheetLength = 6;

        // Skeleton Death
        int animOffset = tomentdatapack.IMGArch.tocOffset + (tomentdatapack.IMGArch.toc[IMG_ID_SPELL_ICE_BLAST_EXPLOSION].startingOffset);
        SDL_RWops* animSdlWops = SDL_RWFromConstMem((byte*)tomentdatapack.IMGArch.buffer+animOffset, tomentdatapack.IMGArch.toc[IMG_ID_SPELL_ICE_BLAST_EXPLOSION].size);
        SDL_Surface* animTemp1 = SDL_LoadBMP_RW(animSdlWops, SDL_TRUE);
        tomentdatapack.sprites[S_IceBlast]->animations->animDie = SDL_ConvertSurface(animTemp1, win_surface->format, SDL_TEXTUREACCESS_TARGET);
        tomentdatapack.sprites[S_IceBlast]->animations->animDieSheetLength = 6;

        SDL_FreeSurface(animTemp1);
    }
    else
        tomentdatapack.sprites[S_IceBlast]->texture = tomentdatapack.enginesDefaults[EDEFAULT_1]->texture;
    U_SetBit(&tomentdatapack.sprites[S_IceBlast]->flags, 0); // Set collision bit flag to 1
    U_SetBit(&tomentdatapack.sprites[S_IceBlast]->flags, 1); // Set animated sprite bit flag to 1
    // Sprite-Specific, set the lookup table for the sprite sheets length
    tomentdatapack.spritesSheetsLenghtTable[S_IceBlast] = 6;

    // Callback
    tomentdatapack.sprites[S_IceBlast]->Callback = NULL;
    SDL_FreeSurface(temp1);

    // AI SkeletonBurnt
    offset = tomentdatapack.IMGArch.tocOffset + (tomentdatapack.IMGArch.toc[IMG_ID_AI_SKELETON_BURNT].startingOffset);
    sdlWops = SDL_RWFromConstMem((byte*)tomentdatapack.IMGArch.buffer+offset, tomentdatapack.IMGArch.toc[IMG_ID_AI_SKELETON_BURNT].size);
    temp1 = SDL_LoadBMP_RW(sdlWops, SDL_TRUE);
    if(D_CheckTextureLoaded(temp1, IMG_ID_AI_SKELETON_BURNT))
    {
        tomentdatapack.sprites[DS_FrozenLordsCaster]->texture = SDL_ConvertSurface(temp1, win_surface->format, SDL_TEXTUREACCESS_TARGET);

        // Load animations as well
        tomentdatapack.sprites[DS_FrozenLordsCaster]->animations = (objectanimations_t*)malloc(sizeof(objectanimations_t));
        D_InitializeObjectAnim(tomentdatapack.sprites[DS_FrozenLordsCaster]->animations);
        tomentdatapack.sprites[DS_FrozenLordsCaster]->animations->belongsTo = tomentdatapack.sprites[DS_FrozenLordsCaster];

        // Idle = Normal
        tomentdatapack.sprites[DS_FrozenLordsCaster]->animations->animIdle = SDL_ConvertSurface(temp1, win_surface->format, SDL_TEXTUREACCESS_TARGET);
        tomentdatapack.sprites[DS_FrozenLordsCaster]->animations->animIdleSheetLength = 0;

        // Skeleton Death
        int animOffset = tomentdatapack.IMGArch.tocOffset + (tomentdatapack.IMGArch.toc[IMG_ID_AI_SKELETON_BURNT_DEATH].startingOffset);
        SDL_RWops* animSdlWops = SDL_RWFromConstMem((byte*)tomentdatapack.IMGArch.buffer+animOffset, tomentdatapack.IMGArch.toc[IMG_ID_AI_SKELETON_BURNT_DEATH].size);
        SDL_Surface* animTemp1 = SDL_LoadBMP_RW(animSdlWops, SDL_TRUE);
        tomentdatapack.sprites[DS_FrozenLordsCaster]->animations->animDie = SDL_ConvertSurface(animTemp1, win_surface->format, SDL_TEXTUREACCESS_TARGET);
        tomentdatapack.sprites[DS_FrozenLordsCaster]->animations->animDieSheetLength = 4;
        SDL_FreeSurface(animTemp1);

        // Skeleton Attack
        animOffset = tomentdatapack.IMGArch.tocOffset + (tomentdatapack.IMGArch.toc[IMG_ID_AI_SKELETON_BURNT_ATTACK2].startingOffset);
        animSdlWops = SDL_RWFromConstMem((byte*)tomentdatapack.IMGArch.buffer+animOffset, tomentdatapack.IMGArch.toc[IMG_ID_AI_SKELETON_BURNT_ATTACK2].size);
        animTemp1 = SDL_LoadBMP_RW(animSdlWops, SDL_TRUE);
        tomentdatapack.sprites[DS_FrozenLordsCaster]->animations->animAttack = SDL_ConvertSurface(animTemp1, win_surface->format, SDL_TEXTUREACCESS_TARGET);
        tomentdatapack.sprites[DS_FrozenLordsCaster]->animations->animAttackSheetLength = 6;
        SDL_FreeSurface(animTemp1);

    }
    else
        tomentdatapack.sprites[DS_FrozenLordsCaster]->texture = tomentdatapack.enginesDefaults[EDEFAULT_1]->texture;
    U_SetBit(&tomentdatapack.sprites[DS_FrozenLordsCaster]->flags, 0); // Set collision bit flag to 1
    U_SetBit(&tomentdatapack.sprites[DS_FrozenLordsCaster]->flags, 2); // Set dynamic bit flag to 1

    // Callback
    tomentdatapack.sprites[DS_FrozenLordsCaster]->Callback = NULL;
    SDL_FreeSurface(temp1);

    // Final sets
    D_SetObject(spritesBarrel1, S_Barrel1, tomentdatapack.sprites[S_Barrel1]->texture, NULL);
    D_SetObject(spritesCampfire, S_Campfire, tomentdatapack.sprites[S_Campfire]->texture, NULL);
    D_SetObject(aiSkeleton, DS_Skeleton, tomentdatapack.sprites[DS_Skeleton]->texture, NULL);
    D_SetObject(pickupAxe, S_PickupAxe, tomentdatapack.sprites[S_PickupAxe]->texture, NULL);
    D_SetObject(pickupHealthPotion, S_PickupHealthPotion, tomentdatapack.sprites[S_PickupHealthPotion]->texture, NULL);
    D_SetObject(pickupManaPotion, S_PickupManaPotion, tomentdatapack.sprites[S_PickupManaPotion]->texture, NULL);
    D_SetObject(spellIceDart1, S_IceDart1, tomentdatapack.sprites[S_IceDart1]->texture, NULL);
    D_SetObject(tomeFireball1, S_TomeFireball1, tomentdatapack.sprites[S_TomeFireball1]->texture, NULL);
    D_SetObject(tomeIceDart1, S_TomeIceDart1, tomentdatapack.sprites[S_TomeIceDart1]->texture, NULL);
    D_SetObject(table1, S_Table1, tomentdatapack.sprites[S_Table1]->texture, NULL);
    D_SetObject(skullStatic, S_SkullStatic, tomentdatapack.sprites[S_SkullStatic]->texture, NULL);
    D_SetObject(altarEmpty, S_AltarEmpty, tomentdatapack.sprites[S_AltarEmpty]->texture, NULL);
    D_SetObject(altarHealth, S_AltarHealth, tomentdatapack.sprites[S_AltarHealth]->texture, NULL);
    D_SetObject(altarMana, S_AltarMana, tomentdatapack.sprites[S_AltarMana]->texture, NULL);
    D_SetObject(aiSkeletonBurnt, DS_SkeletonBurnt, tomentdatapack.sprites[DS_SkeletonBurnt]->texture, NULL);
    D_SetObject(pickupGreatsword, S_PickupGreatsword, tomentdatapack.sprites[S_PickupGreatsword]->texture, NULL);
    D_SetObject(aiSkeletonLord, DS_SkeletonLord, tomentdatapack.sprites[DS_SkeletonLord]->texture, NULL);
    D_SetObject(achtung, S_Achtung, tomentdatapack.sprites[S_Achtung]->texture, NULL);
    D_SetObject(playerCharacterTank, DS_PlayerTank, tomentdatapack.sprites[DS_PlayerTank]->texture, NULL);
    D_SetObject(playerCharacterHealer, DS_PlayerHealer, tomentdatapack.sprites[DS_PlayerHealer]->texture, NULL);
    D_SetObject(playerCharacterDPS, DS_PlayerDPS, tomentdatapack.sprites[DS_PlayerDPS]->texture, NULL);
    D_SetObject(spellConcentratedHeal, S_ConcentratedHeal, tomentdatapack.sprites[S_ConcentratedHeal]->texture, NULL);
    D_SetObject(spellSwordProjectile, S_SwordProjectile, tomentdatapack.sprites[S_SwordProjectile]->texture, NULL);
    D_SetObject(aiMorgathulTheKeeper, DS_MorgathulTheKeeper, tomentdatapack.sprites[DS_MorgathulTheKeeper]->texture, NULL);
    D_SetObject(aiMorgathulCopy, DS_MorgathulCopy, tomentdatapack.sprites[DS_MorgathulCopy]->texture, NULL);
    D_SetObject(altarSpellPower, S_AltarSpellPower, tomentdatapack.sprites[S_AltarSpellPower]->texture, NULL);
    D_SetObject(aiTheFrozenLord, DS_TheFrozenLord, tomentdatapack.sprites[DS_TheFrozenLord]->texture, NULL);
    D_SetObject(pickupChangeEnvironmentTheFrozenEnd, S_PickupChangeEnvironmentTheFrozenEnd, tomentdatapack.sprites[S_PickupChangeEnvironmentTheFrozenEnd]->texture, NULL);
    D_SetObject(spellIceBlast, S_IceBlast, tomentdatapack.sprites[S_IceBlast]->texture, NULL);
    D_SetObject(spellIceBlast, DS_FrozenLordsCaster, tomentdatapack.sprites[DS_FrozenLordsCaster]->texture, NULL);
}


void D_InitFontSheets(void)
{
    fontsheet_t* fontBlckry = (fontsheet_t*)malloc(sizeof(fontsheet_t));
    fontsheet_t* fontBlckrygGreen = (fontsheet_t*)malloc(sizeof(fontsheet_t));

    tomentdatapack.fontsheetsLength = 2;

    tomentdatapack.fontsheets[FONT_BLKCRY] = fontBlckry;
    tomentdatapack.fontsheets[FONT_BLKCRY_GREEN] = fontBlckrygGreen;

    // Convert all the surfaces that we will load in the same format as the win_surface
    SDL_Surface *temp1;     // Surface
    SDL_RWops* sdlWops;     // Structure to read bytes
    int offset;             // Offset in the img.archt

    SDL_Surface* glyphSurface; // Contains a single glyph, used to calculate glyphWidth
    glyphSurface = SDL_CreateRGBSurfaceWithFormat(0, 32, 32, 24, win_surface->format->format);
    SDL_FillRect(glyphSurface, NULL, r_transparencyColor);

    // Load BLCKCRY font
    int width = 32;
    int nHorElements = 16;
    int nVerElements = 6;
    tomentdatapack.fontsheets[FONT_BLKCRY]->width = width;
    tomentdatapack.fontsheets[FONT_BLKCRY]->numHorElements = nHorElements;
    tomentdatapack.fontsheets[FONT_BLKCRY]->numVerElements = nVerElements;

    // Load sheet and put it into texture
    offset = tomentdatapack.IMGArch.tocOffset + (tomentdatapack.IMGArch.toc[IMG_ID_BLKCRY_TEXT_SHEET].startingOffset);
    sdlWops = SDL_RWFromConstMem((byte*)tomentdatapack.IMGArch.buffer+offset, tomentdatapack.IMGArch.toc[IMG_ID_BLKCRY_TEXT_SHEET].size);
    temp1 = SDL_LoadBMP_RW(sdlWops, SDL_TRUE);
    if(D_CheckTextureLoaded(temp1, IMG_ID_BLKCRY_TEXT_SHEET))
    {
        tomentdatapack.fontsheets[FONT_BLKCRY]->texture = SDL_ConvertSurface(temp1, win_surface->format, SDL_TEXTUREACCESS_TARGET);
        SDL_SetColorKey(tomentdatapack.fontsheets[FONT_BLKCRY]->texture, SDL_TRUE, r_transparencyColor);    // Make transparency color for blitting
    }
    else
        tomentdatapack.fontsheets[FONT_BLKCRY]->texture = tomentdatapack.enginesDefaults[EDEFAULT_1]->texture;
    SDL_FreeSurface(temp1);


    // Calculate Glyph Width (used for spacing)
    SDL_Rect glyphSurfaceRect = {0, 0, tomentdatapack.fontsheets[FONT_BLKCRY]->width, tomentdatapack.fontsheets[FONT_BLKCRY]->width};
    // For each element
    for(int y = 0; y < nVerElements; y++)
    {
        for(int x = 0; x < nHorElements; x++)
            {
                // Glyph to analyze
                SDL_Rect srcRect = {x*width, y*width, width, width};
                // Put it in the glyphSurface
                SDL_BlitSurface(tomentdatapack.fontsheets[FONT_BLKCRY]->texture, &srcRect, glyphSurface, NULL);

                // Analyze the glyphSurface and calculate width by reading each line of the glyph and selecting the biggest distance of pixels between the first found and last found
                int start = 0, end = 0, length = 0;
                // For each column of the current glyph element
                for(int gY = 0; gY < width; gY++)
                {
                    bool firstPixel = false;
                    // For each line calculate the width
                    for(int gX = 0; gX < width; gX++)
                        {
                            Uint32 pixel = R_GetPixelFromSurface(glyphSurface, gX, gY);
                            if(pixel != r_transparencyColor)
                            {
                                // If it's the first time we found a non-transparent pixel
                                if(firstPixel == false)
                                {
                                    start = end = gX;
                                    firstPixel = true;
                                }
                                else
                                    end = gX;
                            }

                            // Length is calculated on based on distance, +1 because we're counting the pixels 
                            if((end - start) != 0 && length < (end - start)+1)
                                length = (end - start)+1;

                            // Save length in 
                            tomentdatapack.fontsheets[FONT_BLKCRY]->glyphsWidth[y][x] = length+1;   // +1 because for this font it looks better if each character is given an extra space
                        }                    
                }

                // Override length of first character, it is always the space
                tomentdatapack.fontsheets[FONT_BLKCRY]->glyphsWidth[0][0] = width/4;

                // Reset the glyphSurface
                SDL_FillRect(glyphSurface, NULL, r_transparencyColor);
            }
    }

    // Load BLCKCRY GREEN font
    width = 32;
    nHorElements = 16;
    nVerElements = 6;
    tomentdatapack.fontsheets[FONT_BLKCRY_GREEN]->width = width;
    tomentdatapack.fontsheets[FONT_BLKCRY_GREEN]->numHorElements = nHorElements;
    tomentdatapack.fontsheets[FONT_BLKCRY_GREEN]->numVerElements = nVerElements;

    // Load sheet and put it into texture
    offset = tomentdatapack.IMGArch.tocOffset + (tomentdatapack.IMGArch.toc[IMG_ID_BLKCRY_GREEN_TEXT_SHEET].startingOffset);
    sdlWops = SDL_RWFromConstMem((byte*)tomentdatapack.IMGArch.buffer+offset, tomentdatapack.IMGArch.toc[IMG_ID_BLKCRY_GREEN_TEXT_SHEET].size);
    temp1 = SDL_LoadBMP_RW(sdlWops, SDL_TRUE);
    if(D_CheckTextureLoaded(temp1, IMG_ID_BLKCRY_GREEN_TEXT_SHEET))
    {
        tomentdatapack.fontsheets[FONT_BLKCRY_GREEN]->texture = SDL_ConvertSurface(temp1, win_surface->format, SDL_TEXTUREACCESS_TARGET);
        SDL_SetColorKey(tomentdatapack.fontsheets[FONT_BLKCRY_GREEN]->texture, SDL_TRUE, r_transparencyColor);    // Make transparency color for blitting
    }
    else
        tomentdatapack.fontsheets[FONT_BLKCRY_GREEN]->texture = tomentdatapack.enginesDefaults[EDEFAULT_1]->texture;
    SDL_FreeSurface(temp1);


    // For each element
    for(int y = 0; y < nVerElements; y++)
    {
        for(int x = 0; x < nHorElements; x++)
            {
                // Glyph to analyze
                SDL_Rect srcRect = {x*width, y*width, width, width};
                // Put it in the glyphSurface
                SDL_BlitSurface(tomentdatapack.fontsheets[FONT_BLKCRY_GREEN]->texture, &srcRect, glyphSurface, NULL);

                // Analyze the glyphSurface and calculate width by reading each line of the glyph and selecting the biggest distance of pixels between the first found and last found
                int start = 0, end = 0, length = 0;
                // For each column of the current glyph element
                for(int gY = 0; gY < width; gY++)
                {
                    bool firstPixel = false;
                    // For each line calculate the width
                    for(int gX = 0; gX < width; gX++)
                        {
                            Uint32 pixel = R_GetPixelFromSurface(glyphSurface, gX, gY);
                            if(pixel != r_transparencyColor)
                            {
                                // If it's the first time we found a non-transparent pixel
                                if(firstPixel == false)
                                {
                                    start = end = gX;
                                    firstPixel = true;
                                }
                                else
                                    end = gX;
                            }

                            // Length is calculated on based on distance, +1 because we're counting the pixels 
                            if((end - start) != 0 && length < (end - start)+1)
                                length = (end - start)+1;

                            // Save length in 
                            tomentdatapack.fontsheets[FONT_BLKCRY_GREEN]->glyphsWidth[y][x] = length+1;   // +1 because for this font it looks better if each character is given an extra space
                        }                    
                }

                // Override length of first character, it is always the space
                tomentdatapack.fontsheets[FONT_BLKCRY_GREEN]->glyphsWidth[0][0] = width/4;

                // Reset the glyphSurface
                SDL_FillRect(glyphSurface, NULL, r_transparencyColor);
            }
    }

    // More fonts...

    // This needs to be freed only once, at the end
    SDL_FreeSurface(glyphSurface);
}


void D_InitLoadSkies(void)
{
    // Create Objects
    object_t* skyDefault = (object_t*)malloc(sizeof(object_t));
    object_t* skyRed = (object_t*)malloc(sizeof(object_t));
    object_t* skyIcyMountains = (object_t*)malloc(sizeof(object_t));

    tomentdatapack.skiesLength = 3; // Set length

    D_InitObject(skyDefault);
    D_InitObject(skyRed);
    D_InitObject(skyIcyMountains);

    // Put objects in the datapack
    tomentdatapack.skies[SKY_Default1] = skyDefault;
    tomentdatapack.skies[SKY_Red1] = skyRed;
    tomentdatapack.skies[SKY_Night] = skyRed;

    // Fill objects
    // Convert all the surfaces that we will load in the same format as the win_surface
    SDL_Surface *temp1;     // Surface
    SDL_RWops* sdlWops;     // Structure to read bytes
    int offset;             // Offset in the img.archt

    // Sky Default
    offset = tomentdatapack.IMGArch.tocOffset + (tomentdatapack.IMGArch.toc[IMG_ID_SKY_DEFAULT].startingOffset);
    sdlWops = SDL_RWFromConstMem((byte*)tomentdatapack.IMGArch.buffer+offset, tomentdatapack.IMGArch.toc[IMG_ID_SKY_DEFAULT].size);
    temp1 = SDL_LoadBMP_RW(sdlWops, SDL_TRUE);
    if(D_CheckTextureLoaded(temp1, IMG_ID_SKY_DEFAULT))
        tomentdatapack.skies[SKY_Default1]->texture = SDL_ConvertSurface(temp1, win_surface->format, SDL_TEXTUREACCESS_TARGET);
    else
        tomentdatapack.skies[SKY_Default1]->texture = tomentdatapack.enginesDefaults[EDEFAULT_1]->texture;
    SDL_FreeSurface(temp1);

    // Sky Red
    offset = tomentdatapack.IMGArch.tocOffset + (tomentdatapack.IMGArch.toc[IMG_ID_SKY_RED].startingOffset);
    sdlWops = SDL_RWFromConstMem((byte*)tomentdatapack.IMGArch.buffer+offset, tomentdatapack.IMGArch.toc[IMG_ID_SKY_RED].size);
    temp1 = SDL_LoadBMP_RW(sdlWops, SDL_TRUE);
    if(D_CheckTextureLoaded(temp1, IMG_ID_SKY_RED))
        tomentdatapack.skies[SKY_Red1]->texture = SDL_ConvertSurface(temp1, win_surface->format, SDL_TEXTUREACCESS_TARGET);
    else
        tomentdatapack.skies[SKY_Red1]->texture = tomentdatapack.enginesDefaults[EDEFAULT_1]->texture;
    SDL_FreeSurface(temp1);

    // Sky Icy Mountains
    offset = tomentdatapack.IMGArch.tocOffset + (tomentdatapack.IMGArch.toc[IMG_ID_SKY_NIGHT].startingOffset);
    sdlWops = SDL_RWFromConstMem((byte*)tomentdatapack.IMGArch.buffer+offset, tomentdatapack.IMGArch.toc[IMG_ID_SKY_NIGHT].size);
    temp1 = SDL_LoadBMP_RW(sdlWops, SDL_TRUE);
    if(D_CheckTextureLoaded(temp1, IMG_ID_SKY_NIGHT))
        tomentdatapack.skies[SKY_Night]->texture = SDL_ConvertSurface(temp1, win_surface->format, SDL_TEXTUREACCESS_TARGET);
    else
        tomentdatapack.skies[SKY_Night]->texture = tomentdatapack.enginesDefaults[EDEFAULT_1]->texture;
    SDL_FreeSurface(temp1);

    // Final sets
    D_SetObject(skyDefault, SKY_Default1, tomentdatapack.skies[SKY_Default1]->texture, NULL);
    D_SetObject(skyRed, SKY_Red1, tomentdatapack.skies[SKY_Red1]->texture, NULL);
    D_SetObject(skyRed, SKY_Night, tomentdatapack.skies[SKY_Night]->texture, NULL);
}

void D_InitLoadPlayersFP(void)
{
    // Create Objects
    object_t* playerFPHands = (object_t*)malloc(sizeof(object_t));
    object_t* playerFPAxe = (object_t*)malloc(sizeof(object_t));
    object_t* playerFPGreatsword = (object_t*)malloc(sizeof(object_t));
    object_t* playerFPMace = (object_t*)malloc(sizeof(object_t));

    tomentdatapack.playersFPLength = 4; // Set length

    D_InitObject(playerFPHands);
    D_InitObject(playerFPAxe);
    D_InitObject(playerFPGreatsword);
    D_InitObject(playerFPMace);

    // Put objects in the datapack
    tomentdatapack.playersFP[PLAYER_FP_HANDS] = playerFPHands;
    tomentdatapack.playersFP[PLAYER_FP_AXE] = playerFPAxe;
    tomentdatapack.playersFP[PLAYER_FP_GREATSWORD] = playerFPGreatsword;
    tomentdatapack.playersFP[PLAYER_FP_MACE] = playerFPMace;

    // Fill objects
    // Convert all the surfaces that we will load in the same format as the win_surface
    SDL_Surface *temp1;     // Surface
    SDL_RWops* sdlWops;     // Structure to read bytes
    int offset;             // Offset in the img.archt

    // FP Hands
    offset = tomentdatapack.IMGArch.tocOffset + (tomentdatapack.IMGArch.toc[IMG_ID_P_HANDS_IDLE].startingOffset);
    sdlWops = SDL_RWFromConstMem((byte*)tomentdatapack.IMGArch.buffer+offset, tomentdatapack.IMGArch.toc[IMG_ID_P_HANDS_IDLE].size);
    temp1 = SDL_LoadBMP_RW(sdlWops, SDL_TRUE);
    if(D_CheckTextureLoaded(temp1, IMG_ID_P_HANDS_IDLE))
    {
        tomentdatapack.playersFP[PLAYER_FP_HANDS]->texture = SDL_ConvertSurface(temp1, win_surface->format, SDL_TEXTUREACCESS_TARGET);
        SDL_SetColorKey(tomentdatapack.playersFP[PLAYER_FP_HANDS]->texture, SDL_TRUE, r_transparencyColor);    // Make transparency color for blitting

        // Load animations as well
        tomentdatapack.playersFP[PLAYER_FP_HANDS]->animations = (objectanimations_t*)malloc(sizeof(objectanimations_t));
        D_InitializeObjectAnim(tomentdatapack.playersFP[PLAYER_FP_HANDS]->animations);
        tomentdatapack.playersFP[PLAYER_FP_HANDS]->animations->belongsTo = tomentdatapack.playersFP[0];

        // Idle = Normal
        tomentdatapack.playersFP[PLAYER_FP_HANDS]->animations->animIdle = SDL_ConvertSurface(temp1, win_surface->format, SDL_TEXTUREACCESS_TARGET);
        tomentdatapack.playersFP[PLAYER_FP_HANDS]->animations->animIdleSheetLength = 0;
        tomentdatapack.playersFP[PLAYER_FP_HANDS]->animations->animIdleSpeedModifier = 0;

        SDL_SetColorKey(tomentdatapack.playersFP[PLAYER_FP_HANDS]->animations->animIdle, SDL_TRUE, r_transparencyColor);    // Make transparency color for blitting

        // Attack
        int animOffset = tomentdatapack.IMGArch.tocOffset + (tomentdatapack.IMGArch.toc[IMG_ID_P_HANDS_ATTACK].startingOffset);
        SDL_RWops* animSdlWops = SDL_RWFromConstMem((byte*)tomentdatapack.IMGArch.buffer+animOffset, tomentdatapack.IMGArch.toc[IMG_ID_P_HANDS_ATTACK].size);
        SDL_Surface* animTemp1 = SDL_LoadBMP_RW(animSdlWops, SDL_TRUE);
        tomentdatapack.playersFP[PLAYER_FP_HANDS]->animations->animAttack = SDL_ConvertSurface(animTemp1, win_surface->format, SDL_TEXTUREACCESS_TARGET);
        tomentdatapack.playersFP[PLAYER_FP_HANDS]->animations->animAttackSheetLength = 4;
        tomentdatapack.playersFP[PLAYER_FP_HANDS]->animations->animAttackActionFrame = 3;
        tomentdatapack.playersFP[PLAYER_FP_HANDS]->animations->animAttackSpeedModifier = 0;
        SDL_SetColorKey(tomentdatapack.playersFP[PLAYER_FP_HANDS]->animations->animAttack, SDL_TRUE, r_transparencyColor);    // Make transparency color for blitting
        SDL_FreeSurface(animTemp1);

        // Cast Spell
        animOffset = tomentdatapack.IMGArch.tocOffset + (tomentdatapack.IMGArch.toc[IMG_ID_P_HANDS_CASTSPELL].startingOffset);
        animSdlWops = SDL_RWFromConstMem((byte*)tomentdatapack.IMGArch.buffer+animOffset, tomentdatapack.IMGArch.toc[IMG_ID_P_HANDS_CASTSPELL].size);
        animTemp1 = SDL_LoadBMP_RW(animSdlWops, SDL_TRUE);
        tomentdatapack.playersFP[PLAYER_FP_HANDS]->animations->animCastSpell = SDL_ConvertSurface(animTemp1, win_surface->format, SDL_TEXTUREACCESS_TARGET);
        tomentdatapack.playersFP[PLAYER_FP_HANDS]->animations->animCastSpellSheetLength = 6;
        tomentdatapack.playersFP[PLAYER_FP_HANDS]->animations->animCastSpellActionFrame = 4;
        tomentdatapack.playersFP[PLAYER_FP_HANDS]->animations->animCastSpellkSpeedModifier = 0;
        SDL_SetColorKey(tomentdatapack.playersFP[PLAYER_FP_HANDS]->animations->animCastSpell, SDL_TRUE, r_transparencyColor);    // Make transparency color for blitting
        SDL_FreeSurface(animTemp1);

    }
    else
        tomentdatapack.playersFP[PLAYER_FP_HANDS]->texture = tomentdatapack.enginesDefaults[EDEFAULT_1]->texture;
    SDL_FreeSurface(temp1);

    // FP Axe
    offset = tomentdatapack.IMGArch.tocOffset + (tomentdatapack.IMGArch.toc[IMG_ID_P_AXE_IDLE].startingOffset);
    sdlWops = SDL_RWFromConstMem((byte*)tomentdatapack.IMGArch.buffer+offset, tomentdatapack.IMGArch.toc[IMG_ID_P_AXE_IDLE].size);
    temp1 = SDL_LoadBMP_RW(sdlWops, SDL_TRUE);
    if(D_CheckTextureLoaded(temp1, IMG_ID_P_AXE_IDLE))
    {
        tomentdatapack.playersFP[PLAYER_FP_AXE]->texture = SDL_ConvertSurface(temp1, win_surface->format, SDL_TEXTUREACCESS_TARGET);
        SDL_SetColorKey(tomentdatapack.playersFP[PLAYER_FP_AXE]->texture, SDL_TRUE, r_transparencyColor);    // Make transparency color for blitting

        // Load animations as well
        tomentdatapack.playersFP[PLAYER_FP_AXE]->animations = (objectanimations_t*)malloc(sizeof(objectanimations_t));
        D_InitializeObjectAnim(tomentdatapack.playersFP[PLAYER_FP_AXE]->animations);
        tomentdatapack.playersFP[PLAYER_FP_AXE]->animations->belongsTo = tomentdatapack.playersFP[0];

        // Idle = Normal
        tomentdatapack.playersFP[PLAYER_FP_AXE]->animations->animIdle = SDL_ConvertSurface(temp1, win_surface->format, SDL_TEXTUREACCESS_TARGET);
        tomentdatapack.playersFP[PLAYER_FP_AXE]->animations->animIdleSheetLength = 0;
        tomentdatapack.playersFP[PLAYER_FP_AXE]->animations->animIdleSpeedModifier = 0;

        SDL_SetColorKey(tomentdatapack.playersFP[PLAYER_FP_AXE]->animations->animIdle, SDL_TRUE, r_transparencyColor);    // Make transparency color for blitting

        // Attack
        int animOffset = tomentdatapack.IMGArch.tocOffset + (tomentdatapack.IMGArch.toc[IMG_ID_P_AXE_ATTACK1].startingOffset);
        SDL_RWops* animSdlWops = SDL_RWFromConstMem((byte*)tomentdatapack.IMGArch.buffer+animOffset, tomentdatapack.IMGArch.toc[IMG_ID_P_AXE_ATTACK1].size);
        SDL_Surface* animTemp1 = SDL_LoadBMP_RW(animSdlWops, SDL_TRUE);
        tomentdatapack.playersFP[PLAYER_FP_AXE]->animations->animAttack = SDL_ConvertSurface(animTemp1, win_surface->format, SDL_TEXTUREACCESS_TARGET);
        tomentdatapack.playersFP[PLAYER_FP_AXE]->animations->animAttackSheetLength = 6;
        tomentdatapack.playersFP[PLAYER_FP_AXE]->animations->animAttackActionFrame = 3;
        tomentdatapack.playersFP[PLAYER_FP_AXE]->animations->animAttackSpeedModifier = 0;
        SDL_SetColorKey(tomentdatapack.playersFP[PLAYER_FP_AXE]->animations->animAttack, SDL_TRUE, r_transparencyColor);    // Make transparency color for blitting
        SDL_FreeSurface(animTemp1);

        // Cast Spell
        animOffset = tomentdatapack.IMGArch.tocOffset + (tomentdatapack.IMGArch.toc[IMG_ID_P_HANDS_CASTSPELL].startingOffset);
        animSdlWops = SDL_RWFromConstMem((byte*)tomentdatapack.IMGArch.buffer+animOffset, tomentdatapack.IMGArch.toc[IMG_ID_P_HANDS_CASTSPELL].size);
        animTemp1 = SDL_LoadBMP_RW(animSdlWops, SDL_TRUE);
        tomentdatapack.playersFP[PLAYER_FP_AXE]->animations->animCastSpell = SDL_ConvertSurface(animTemp1, win_surface->format, SDL_TEXTUREACCESS_TARGET);
        tomentdatapack.playersFP[PLAYER_FP_AXE]->animations->animCastSpellSheetLength = 6;
        tomentdatapack.playersFP[PLAYER_FP_AXE]->animations->animCastSpellActionFrame = 4;
        tomentdatapack.playersFP[PLAYER_FP_AXE]->animations->animCastSpellkSpeedModifier = 0;
        SDL_SetColorKey(tomentdatapack.playersFP[PLAYER_FP_AXE]->animations->animCastSpell, SDL_TRUE, r_transparencyColor);    // Make transparency color for blitting
        SDL_FreeSurface(animTemp1);

        // Healer skill 1 -  SelfHeal
        animOffset = tomentdatapack.IMGArch.tocOffset + (tomentdatapack.IMGArch.toc[IMG_ID_SKILL_ANIM_HEALER_SELF_HEAL].startingOffset);
        animSdlWops = SDL_RWFromConstMem((byte*)tomentdatapack.IMGArch.buffer+animOffset, tomentdatapack.IMGArch.toc[IMG_ID_SKILL_ANIM_HEALER_SELF_HEAL].size);
        animTemp1 = SDL_LoadBMP_RW(animSdlWops, SDL_TRUE);
        tomentdatapack.playersFP[PLAYER_FP_AXE]->animations->animSpecial1 = SDL_ConvertSurface(animTemp1, win_surface->format, SDL_TEXTUREACCESS_TARGET);
        tomentdatapack.playersFP[PLAYER_FP_AXE]->animations->animSpecial1SheetLength = 6;
        tomentdatapack.playersFP[PLAYER_FP_AXE]->animations->animSpecial1ActionFrame = 3;
        tomentdatapack.playersFP[PLAYER_FP_AXE]->animations->animSpecial1SpeedModifier = 0;
        SDL_SetColorKey(tomentdatapack.playersFP[PLAYER_FP_AXE]->animations->animSpecial1, SDL_TRUE, r_transparencyColor);    // Make transparency color for blitting
        SDL_FreeSurface(animTemp1);

        // Cast Spell
        animOffset = tomentdatapack.IMGArch.tocOffset + (tomentdatapack.IMGArch.toc[IMG_ID_P_HANDS_CASTSPELL].startingOffset);
        animSdlWops = SDL_RWFromConstMem((byte*)tomentdatapack.IMGArch.buffer+animOffset, tomentdatapack.IMGArch.toc[IMG_ID_P_HANDS_CASTSPELL].size);
        animTemp1 = SDL_LoadBMP_RW(animSdlWops, SDL_TRUE);
        tomentdatapack.playersFP[PLAYER_FP_AXE]->animations->animSpecial2 = SDL_ConvertSurface(animTemp1, win_surface->format, SDL_TEXTUREACCESS_TARGET);
        tomentdatapack.playersFP[PLAYER_FP_AXE]->animations->animSpecial2SheetLength = 6;
        tomentdatapack.playersFP[PLAYER_FP_AXE]->animations->animSpecial2ActionFrame = 4;
        tomentdatapack.playersFP[PLAYER_FP_AXE]->animations->animSpecial2SpeedModifier = 0;
        SDL_SetColorKey(tomentdatapack.playersFP[PLAYER_FP_AXE]->animations->animSpecial2, SDL_TRUE, r_transparencyColor);    // Make transparency color for blitting
        SDL_FreeSurface(animTemp1);

        // Healer skill 1 -  SelfHeal
        animOffset = tomentdatapack.IMGArch.tocOffset + (tomentdatapack.IMGArch.toc[IMG_ID_SKILL_ANIM_HEALER_SELF_HEAL].startingOffset);
        animSdlWops = SDL_RWFromConstMem((byte*)tomentdatapack.IMGArch.buffer+animOffset, tomentdatapack.IMGArch.toc[IMG_ID_SKILL_ANIM_HEALER_SELF_HEAL].size);
        animTemp1 = SDL_LoadBMP_RW(animSdlWops, SDL_TRUE);
        tomentdatapack.playersFP[PLAYER_FP_AXE]->animations->animSpecial3 = SDL_ConvertSurface(animTemp1, win_surface->format, SDL_TEXTUREACCESS_TARGET);
        tomentdatapack.playersFP[PLAYER_FP_AXE]->animations->animSpecial3SheetLength = 6;
        tomentdatapack.playersFP[PLAYER_FP_AXE]->animations->animSpecial3ActionFrame = 3;
        tomentdatapack.playersFP[PLAYER_FP_AXE]->animations->animSpecial3SpeedModifier = 0;
        SDL_SetColorKey(tomentdatapack.playersFP[PLAYER_FP_AXE]->animations->animSpecial3, SDL_TRUE, r_transparencyColor);    // Make transparency color for blitting
        SDL_FreeSurface(animTemp1);

    }
    else
        tomentdatapack.playersFP[PLAYER_FP_AXE]->texture = tomentdatapack.enginesDefaults[EDEFAULT_1]->texture;
    SDL_FreeSurface(temp1);

    // FP Greatsword
    offset = tomentdatapack.IMGArch.tocOffset + (tomentdatapack.IMGArch.toc[IMG_ID_P_GREATSWORD_IDLE].startingOffset);
    sdlWops = SDL_RWFromConstMem((byte*)tomentdatapack.IMGArch.buffer+offset, tomentdatapack.IMGArch.toc[IMG_ID_P_GREATSWORD_IDLE].size);
    temp1 = SDL_LoadBMP_RW(sdlWops, SDL_TRUE);
    if(D_CheckTextureLoaded(temp1, IMG_ID_P_GREATSWORD_IDLE))
    {
        tomentdatapack.playersFP[PLAYER_FP_GREATSWORD]->texture = SDL_ConvertSurface(temp1, win_surface->format, SDL_TEXTUREACCESS_TARGET);
        SDL_SetColorKey(tomentdatapack.playersFP[PLAYER_FP_GREATSWORD]->texture, SDL_TRUE, r_transparencyColor);    // Make transparency color for blitting

        // Load animations as well
        tomentdatapack.playersFP[PLAYER_FP_GREATSWORD]->animations = (objectanimations_t*)malloc(sizeof(objectanimations_t));
        D_InitializeObjectAnim(tomentdatapack.playersFP[PLAYER_FP_GREATSWORD]->animations);
        tomentdatapack.playersFP[PLAYER_FP_GREATSWORD]->animations->belongsTo = tomentdatapack.playersFP[0];

        // Idle = Normal
        tomentdatapack.playersFP[PLAYER_FP_GREATSWORD]->animations->animIdle = SDL_ConvertSurface(temp1, win_surface->format, SDL_TEXTUREACCESS_TARGET);
        tomentdatapack.playersFP[PLAYER_FP_GREATSWORD]->animations->animIdleSheetLength = 0;
        tomentdatapack.playersFP[PLAYER_FP_GREATSWORD]->animations->animIdleSpeedModifier = 0;

        SDL_SetColorKey(tomentdatapack.playersFP[PLAYER_FP_GREATSWORD]->animations->animIdle, SDL_TRUE, r_transparencyColor);    // Make transparency color for blitting

        // Attack
        int animOffset = tomentdatapack.IMGArch.tocOffset + (tomentdatapack.IMGArch.toc[IMG_ID_P_GREATSWORD_ATTACK].startingOffset);
        SDL_RWops* animSdlWops = SDL_RWFromConstMem((byte*)tomentdatapack.IMGArch.buffer+animOffset, tomentdatapack.IMGArch.toc[IMG_ID_P_GREATSWORD_ATTACK].size);
        SDL_Surface* animTemp1 = SDL_LoadBMP_RW(animSdlWops, SDL_TRUE);
        tomentdatapack.playersFP[PLAYER_FP_GREATSWORD]->animations->animAttack = SDL_ConvertSurface(animTemp1, win_surface->format, SDL_TEXTUREACCESS_TARGET);
        tomentdatapack.playersFP[PLAYER_FP_GREATSWORD]->animations->animAttackSheetLength = 7;
        tomentdatapack.playersFP[PLAYER_FP_GREATSWORD]->animations->animAttackActionFrame = 3;
        tomentdatapack.playersFP[PLAYER_FP_GREATSWORD]->animations->animAttackSpeedModifier = 0;
        SDL_SetColorKey(tomentdatapack.playersFP[PLAYER_FP_GREATSWORD]->animations->animAttack, SDL_TRUE, r_transparencyColor);    // Make transparency color for blitting
        SDL_FreeSurface(animTemp1);

        // Cast Spell
        animOffset = tomentdatapack.IMGArch.tocOffset + (tomentdatapack.IMGArch.toc[IMG_ID_P_HANDS_CASTSPELL].startingOffset);
        animSdlWops = SDL_RWFromConstMem((byte*)tomentdatapack.IMGArch.buffer+animOffset, tomentdatapack.IMGArch.toc[IMG_ID_P_HANDS_CASTSPELL].size);
        animTemp1 = SDL_LoadBMP_RW(animSdlWops, SDL_TRUE);
        tomentdatapack.playersFP[PLAYER_FP_GREATSWORD]->animations->animCastSpell = SDL_ConvertSurface(animTemp1, win_surface->format, SDL_TEXTUREACCESS_TARGET);
        tomentdatapack.playersFP[PLAYER_FP_GREATSWORD]->animations->animCastSpellSheetLength = 6;
        tomentdatapack.playersFP[PLAYER_FP_GREATSWORD]->animations->animCastSpellActionFrame = 4;
        tomentdatapack.playersFP[PLAYER_FP_GREATSWORD]->animations->animCastSpellkSpeedModifier = 0;
        SDL_SetColorKey(tomentdatapack.playersFP[PLAYER_FP_GREATSWORD]->animations->animCastSpell, SDL_TRUE, r_transparencyColor);    // Make transparency color for blitting
        SDL_FreeSurface(animTemp1);

        // DPS Skill Cheapshot
        animOffset = tomentdatapack.IMGArch.tocOffset + (tomentdatapack.IMGArch.toc[IMG_ID_SKILL_ANIM_DPS_CHEAPSHOT].startingOffset);
        animSdlWops = SDL_RWFromConstMem((byte*)tomentdatapack.IMGArch.buffer+animOffset, tomentdatapack.IMGArch.toc[IMG_ID_SKILL_ANIM_DPS_CHEAPSHOT].size);
        animTemp1 = SDL_LoadBMP_RW(animSdlWops, SDL_TRUE);
        tomentdatapack.playersFP[PLAYER_FP_GREATSWORD]->animations->animSpecial1 = SDL_ConvertSurface(animTemp1, win_surface->format, SDL_TEXTUREACCESS_TARGET);
        tomentdatapack.playersFP[PLAYER_FP_GREATSWORD]->animations->animSpecial1SheetLength = 5;
        tomentdatapack.playersFP[PLAYER_FP_GREATSWORD]->animations->animSpecial1ActionFrame = 2;
        tomentdatapack.playersFP[PLAYER_FP_GREATSWORD]->animations->animSpecial1SpeedModifier = 0;
        SDL_SetColorKey(tomentdatapack.playersFP[PLAYER_FP_GREATSWORD]->animations->animSpecial1, SDL_TRUE, r_transparencyColor);    // Make transparency color for blitting
        SDL_FreeSurface(animTemp1);

        // DPS Skill obliterate
        animOffset = tomentdatapack.IMGArch.tocOffset + (tomentdatapack.IMGArch.toc[IMG_ID_P_GREATSWORD_ATTACK].startingOffset);
        animSdlWops = SDL_RWFromConstMem((byte*)tomentdatapack.IMGArch.buffer+animOffset, tomentdatapack.IMGArch.toc[IMG_ID_P_GREATSWORD_ATTACK].size);
        animTemp1 = SDL_LoadBMP_RW(animSdlWops, SDL_TRUE);
        tomentdatapack.playersFP[PLAYER_FP_GREATSWORD]->animations->animSpecial2 = SDL_ConvertSurface(animTemp1, win_surface->format, SDL_TEXTUREACCESS_TARGET);
        tomentdatapack.playersFP[PLAYER_FP_GREATSWORD]->animations->animSpecial2SheetLength = 7;
        tomentdatapack.playersFP[PLAYER_FP_GREATSWORD]->animations->animSpecial2ActionFrame = 3;
        tomentdatapack.playersFP[PLAYER_FP_GREATSWORD]->animations->animSpecial2SpeedModifier = 0;
        SDL_SetColorKey(tomentdatapack.playersFP[PLAYER_FP_GREATSWORD]->animations->animSpecial2, SDL_TRUE, r_transparencyColor);    // Make transparency color for blitting
        SDL_FreeSurface(animTemp1);

        // DPS Skill split
        animOffset = tomentdatapack.IMGArch.tocOffset + (tomentdatapack.IMGArch.toc[IMG_ID_SKILL_ANIM_DPS_SPLIT].startingOffset);
        animSdlWops = SDL_RWFromConstMem((byte*)tomentdatapack.IMGArch.buffer+animOffset, tomentdatapack.IMGArch.toc[IMG_ID_SKILL_ANIM_DPS_SPLIT].size);
        animTemp1 = SDL_LoadBMP_RW(animSdlWops, SDL_TRUE);
        tomentdatapack.playersFP[PLAYER_FP_GREATSWORD]->animations->animSpecial3 = SDL_ConvertSurface(animTemp1, win_surface->format, SDL_TEXTUREACCESS_TARGET);
        tomentdatapack.playersFP[PLAYER_FP_GREATSWORD]->animations->animSpecial3SheetLength = 7;
        tomentdatapack.playersFP[PLAYER_FP_GREATSWORD]->animations->animSpecial3ActionFrame = 4;
        tomentdatapack.playersFP[PLAYER_FP_GREATSWORD]->animations->animSpecial3SpeedModifier = 0;
        SDL_SetColorKey(tomentdatapack.playersFP[PLAYER_FP_GREATSWORD]->animations->animSpecial3, SDL_TRUE, r_transparencyColor);    // Make transparency color for blitting
        SDL_FreeSurface(animTemp1);
    }
    else
        tomentdatapack.playersFP[PLAYER_FP_GREATSWORD]->texture = tomentdatapack.enginesDefaults[EDEFAULT_1]->texture;
    SDL_FreeSurface(temp1);

    // FP Mace
    offset = tomentdatapack.IMGArch.tocOffset + (tomentdatapack.IMGArch.toc[IMG_ID_P_MACE_IDLE].startingOffset);
    sdlWops = SDL_RWFromConstMem((byte*)tomentdatapack.IMGArch.buffer+offset, tomentdatapack.IMGArch.toc[IMG_ID_P_MACE_IDLE].size);
    temp1 = SDL_LoadBMP_RW(sdlWops, SDL_TRUE);
    if(D_CheckTextureLoaded(temp1, IMG_ID_P_MACE_IDLE))
    {
        tomentdatapack.playersFP[PLAYER_FP_MACE]->texture = SDL_ConvertSurface(temp1, win_surface->format, SDL_TEXTUREACCESS_TARGET);
        SDL_SetColorKey(tomentdatapack.playersFP[PLAYER_FP_MACE]->texture, SDL_TRUE, r_transparencyColor);    // Make transparency color for blitting

        // Load animations as well
        tomentdatapack.playersFP[PLAYER_FP_MACE]->animations = (objectanimations_t*)malloc(sizeof(objectanimations_t));
        D_InitializeObjectAnim(tomentdatapack.playersFP[PLAYER_FP_MACE]->animations);
        tomentdatapack.playersFP[PLAYER_FP_MACE]->animations->belongsTo = tomentdatapack.playersFP[0];

        // Idle = Normal
        tomentdatapack.playersFP[PLAYER_FP_MACE]->animations->animIdle = SDL_ConvertSurface(temp1, win_surface->format, SDL_TEXTUREACCESS_TARGET);
        tomentdatapack.playersFP[PLAYER_FP_MACE]->animations->animIdleSheetLength = 0;
        tomentdatapack.playersFP[PLAYER_FP_MACE]->animations->animIdleSpeedModifier = 0;

        SDL_SetColorKey(tomentdatapack.playersFP[PLAYER_FP_MACE]->animations->animIdle, SDL_TRUE, r_transparencyColor);    // Make transparency color for blitting

        // Attack
        int animOffset = tomentdatapack.IMGArch.tocOffset + (tomentdatapack.IMGArch.toc[IMG_ID_P_MACE_ATTACK1].startingOffset);
        SDL_RWops* animSdlWops = SDL_RWFromConstMem((byte*)tomentdatapack.IMGArch.buffer+animOffset, tomentdatapack.IMGArch.toc[IMG_ID_P_MACE_ATTACK1].size);
        SDL_Surface* animTemp1 = SDL_LoadBMP_RW(animSdlWops, SDL_TRUE);
        tomentdatapack.playersFP[PLAYER_FP_MACE]->animations->animAttack = SDL_ConvertSurface(animTemp1, win_surface->format, SDL_TEXTUREACCESS_TARGET);
        tomentdatapack.playersFP[PLAYER_FP_MACE]->animations->animAttackSheetLength = 6;
        tomentdatapack.playersFP[PLAYER_FP_MACE]->animations->animAttackActionFrame = 3;
        tomentdatapack.playersFP[PLAYER_FP_MACE]->animations->animAttackSpeedModifier = 0;
        SDL_SetColorKey(tomentdatapack.playersFP[PLAYER_FP_MACE]->animations->animAttack, SDL_TRUE, r_transparencyColor);    // Make transparency color for blitting
        SDL_FreeSurface(animTemp1);

        // Cast Spell
        animOffset = tomentdatapack.IMGArch.tocOffset + (tomentdatapack.IMGArch.toc[IMG_ID_P_HANDS_CASTSPELL].startingOffset);
        animSdlWops = SDL_RWFromConstMem((byte*)tomentdatapack.IMGArch.buffer+animOffset, tomentdatapack.IMGArch.toc[IMG_ID_P_HANDS_CASTSPELL].size);
        animTemp1 = SDL_LoadBMP_RW(animSdlWops, SDL_TRUE);
        tomentdatapack.playersFP[PLAYER_FP_MACE]->animations->animCastSpell = SDL_ConvertSurface(animTemp1, win_surface->format, SDL_TEXTUREACCESS_TARGET);
        tomentdatapack.playersFP[PLAYER_FP_MACE]->animations->animCastSpellSheetLength = 6;
        tomentdatapack.playersFP[PLAYER_FP_MACE]->animations->animCastSpellActionFrame = 4;
        tomentdatapack.playersFP[PLAYER_FP_MACE]->animations->animCastSpellkSpeedModifier = 0;
        SDL_SetColorKey(tomentdatapack.playersFP[PLAYER_FP_MACE]->animations->animCastSpell, SDL_TRUE, r_transparencyColor);    // Make transparency color for blitting
        SDL_FreeSurface(animTemp1);

        // Tank skill 1 - Shield Slam
        animOffset = tomentdatapack.IMGArch.tocOffset + (tomentdatapack.IMGArch.toc[IMG_ID_SKILL_ANIM_TANK_SHIELD_SLAM].startingOffset);
        animSdlWops = SDL_RWFromConstMem((byte*)tomentdatapack.IMGArch.buffer+animOffset, tomentdatapack.IMGArch.toc[IMG_ID_SKILL_ANIM_TANK_SHIELD_SLAM].size);
        animTemp1 = SDL_LoadBMP_RW(animSdlWops, SDL_TRUE);
        tomentdatapack.playersFP[PLAYER_FP_MACE]->animations->animSpecial1 = SDL_ConvertSurface(animTemp1, win_surface->format, SDL_TEXTUREACCESS_TARGET);
        tomentdatapack.playersFP[PLAYER_FP_MACE]->animations->animSpecial1SheetLength = 7;
        tomentdatapack.playersFP[PLAYER_FP_MACE]->animations->animSpecial1ActionFrame = 4;
        tomentdatapack.playersFP[PLAYER_FP_MACE]->animations->animSpecial1SpeedModifier = 0;
        SDL_SetColorKey(tomentdatapack.playersFP[PLAYER_FP_MACE]->animations->animSpecial1, SDL_TRUE, r_transparencyColor);    // Make transparency color for blitting
        SDL_FreeSurface(animTemp1);

        // Tank skill 2 - Shield Block
        animOffset = tomentdatapack.IMGArch.tocOffset + (tomentdatapack.IMGArch.toc[IMG_ID_SKILL_ANIM_TANK_SHIELD_BLOCK].startingOffset);
        animSdlWops = SDL_RWFromConstMem((byte*)tomentdatapack.IMGArch.buffer+animOffset, tomentdatapack.IMGArch.toc[IMG_ID_SKILL_ANIM_TANK_SHIELD_BLOCK].size);
        animTemp1 = SDL_LoadBMP_RW(animSdlWops, SDL_TRUE);
        tomentdatapack.playersFP[PLAYER_FP_MACE]->animations->animSpecial2 = SDL_ConvertSurface(animTemp1, win_surface->format, SDL_TEXTUREACCESS_TARGET);
        tomentdatapack.playersFP[PLAYER_FP_MACE]->animations->animSpecial2SheetLength = 7;
        tomentdatapack.playersFP[PLAYER_FP_MACE]->animations->animSpecial2ActionFrame = 0;
        tomentdatapack.playersFP[PLAYER_FP_MACE]->animations->animSpecial2SpeedModifier = 200;
        SDL_SetColorKey(tomentdatapack.playersFP[PLAYER_FP_MACE]->animations->animSpecial2, SDL_TRUE, r_transparencyColor);    // Make transparency color for blitting
        SDL_FreeSurface(animTemp1);

        // Tank skill 3 - Consacration
        animOffset = tomentdatapack.IMGArch.tocOffset + (tomentdatapack.IMGArch.toc[IMG_ID_SKILL_ANIM_TANK_SHIELD_SLAM].startingOffset);
        animSdlWops = SDL_RWFromConstMem((byte*)tomentdatapack.IMGArch.buffer+animOffset, tomentdatapack.IMGArch.toc[IMG_ID_SKILL_ANIM_TANK_SHIELD_SLAM].size);
        animTemp1 = SDL_LoadBMP_RW(animSdlWops, SDL_TRUE);
        tomentdatapack.playersFP[PLAYER_FP_MACE]->animations->animSpecial3 = SDL_ConvertSurface(animTemp1, win_surface->format, SDL_TEXTUREACCESS_TARGET);
        tomentdatapack.playersFP[PLAYER_FP_MACE]->animations->animSpecial3SheetLength = 7;
        tomentdatapack.playersFP[PLAYER_FP_MACE]->animations->animSpecial3ActionFrame = 4;
        tomentdatapack.playersFP[PLAYER_FP_MACE]->animations->animSpecial3SpeedModifier = 0;
        SDL_SetColorKey(tomentdatapack.playersFP[PLAYER_FP_MACE]->animations->animSpecial3, SDL_TRUE, r_transparencyColor);    // Make transparency color for blitting
        SDL_FreeSurface(animTemp1);
    }
    else
        tomentdatapack.playersFP[PLAYER_FP_MACE]->texture = tomentdatapack.enginesDefaults[EDEFAULT_1]->texture;
    SDL_FreeSurface(temp1);

    // Final sets
    D_SetObject(playerFPHands, PLAYER_FP_HANDS, tomentdatapack.playersFP[PLAYER_FP_HANDS]->texture, NULL);
    D_SetObject(playerFPAxe, PLAYER_FP_AXE, tomentdatapack.playersFP[PLAYER_FP_AXE]->texture, NULL);
    D_SetObject(playerFPGreatsword, PLAYER_FP_GREATSWORD, tomentdatapack.playersFP[PLAYER_FP_GREATSWORD]->texture, NULL);
    D_SetObject(playerFPMace, PLAYER_FP_MACE, tomentdatapack.playersFP[PLAYER_FP_MACE]->texture, NULL);
}

//-------------------------------------
// Closes the archives to and frees buffers
//-------------------------------------
void D_CloseArchs(void)
{
    fclose(tomentdatapack.IMGArch.file);
    free(tomentdatapack.IMGArch.buffer);
}
