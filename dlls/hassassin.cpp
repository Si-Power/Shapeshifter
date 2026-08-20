/***
*
*	Copyright (c) 1996-2002, Valve LLC. All rights reserved.
*	
*	This product contains software technology licensed from Id 
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc. 
*	All Rights Reserved.
*
*   This source code contains proprietary and confidential information of
*   Valve LLC and its suppliers.  Access to this code is restricted to
*   persons who have executed a written SDK license with Valve.  Any access,
*   use or distribution of this code by or to any unlicensed person is illegal.
*
****/
#if !defined( OEM_BUILD ) && !defined( HLDEMO_BUILD )

//=========================================================
// hassassin - Human assassin, fast and stealthy
//=========================================================

#include	"extdll.h"
#include	"util.h"
#include	"cbase.h"
#include	"monsters.h"
#include	"schedule.h"
#include	"squadmonster.h"
#include	"weapons.h"
#include	"soundent.h"
#include	"game.h"

#include "animation.h"
#include	"player.h"


#define BaseClass CBaseMonster


extern DLL_GLOBAL int  g_iSkillLevel;


// Глобальные переменные
CBaseEntity* g_pAssassin = NULL;
bool g_bIsActive = false;
Vector g_StoredCameraAngles;


// Кастомный маленький hull для камеры (как у снарядов)
Vector VEC_CAMERA_HULL_MIN(-2, -2, -2);
Vector VEC_CAMERA_HULL_MAX(2, 2, 2);
int cam_hull = 0; // Будем использовать кастомный hull


extern short g_sModelIndexLaser;
extern cvar_t debug_traceline;


extern void Assassin_PlayerTriggeredJump(void);


// Глобальные переменные для системы заряда прыжка
float g_flJumpChargeStartTime = 0;
float g_flJumpChargeAmount = 0;
bool g_bJumpCharging = false;


// Параметры прыжка (настроены для высокого вертикального прыжка)
float g_flMaxJumpChargeTime = 1.0f;        // Максимальное время заряда
float g_flMaxJumpVelocity = 600.0f;          // Максимальная сила прыжка (ОЧЕНЬ ВЫСОКО)  
float g_flMinJumpVelocity = 200.0f;           // Минимальная сила прыжка (уже высоко)
float g_flJumpCooldown = 1.0f;             // Кулдаун между прыжками


// Добавляем в секцию объявлений функций (после существующих объявлений)
extern void Assassin_PlayJumpSound(void);
extern void Assassin_PlayLandSound(void);
extern void Assassin_SetAnimation(const char* animationName);
extern int Assassin_GetCurrentSequence(void);
extern int Assassin_LookupSequence(const char* animationName);
extern bool Assassin_IsOnGround(void);
extern float Assassin_GetVelocityZ(void);
extern bool Assassin_CanSeeEnemy(void);
extern void Assassin_PlayLandAnimation(void);


extern int Assassin_GetSequence(void);
extern void Assassin_SetSequence(int sequence);
extern int Assassin_LookupActivity(int activity);


// Глобальные переменные для системы движения
bool g_bMoveForward = false;
bool g_bMoveBackward = false;
bool g_bMoveLeft = false;
bool g_bMoveRight = false;
float g_flMoveSpeed = 380.0f; // Базовая скорость
extern void Assassin_StopMove(void);


extern bool Assassin_IsOnGround(void);
extern float Assassin_GetVelocityZ(void);
extern int Assassin_GetSequence(void);
extern void Assassin_SetActivity(int activity);

extern void Assassin_RestoreCollisions(void);


// Объявления новых функций
extern void Assassin_Move(void);
extern void Assassin_AirControl(Vector moveDirection);

// Объявления глобальных переменных
extern bool g_bMoveBackward;
extern bool g_bMoveLeft;
extern bool g_bMoveRight;


extern void Assassin_MoveWithJerk(void);
extern Vector CalculateMoveDirection(void);


extern void Assassin_PhysicsInteraction(void);
extern void HandleEntityInteraction(CBaseEntity* pEntity, const char* direction);
extern void Assassin_CheckObjectInteraction(void);


bool g_bAssassinUsePressed = false;
bool g_bAssassinUseHandled = false;
float g_flLastAssassinUse = 0;
float g_flUseCooldown = 0.5f; // Защита от спама

extern bool Assassin_CanUse(void);
extern void Assassin_CheckUse(void);
extern void Assassin_ResetUse(void);

CBaseEntity* Assassin_FindUseEntity(void);


void StopHassassinCamera(CBasePlayer* pPlayer);
void UpdateAssassinHealthSync(CBasePlayer* pPlayer);

float g_flDesiredCameraDistance = 120.0f;
float g_flCurrentCameraDistance = 120.0f;
float g_flCurrentObstructionDistance = 120.0f;
bool g_bCameraObstructed = false;
Vector g_vecLastGoodCameraPos;


bool g_bAssassinControlled = false; // ★★★★ ФЛАГ УПРАВЛЕНИЯ АССАСИНОМ ★★★★


float g_flAssassinDeathTime = 0.0f;    // Время когда ассасин умер
bool g_bAssassinDeathSequence = false; // Флаг что проигрывается анимация смерти
bool g_bAssassinDeathHandled = false;


Vector g_vecDeathCameraOffset;    // Смещение камеры относительно ассасина
float g_flDeathCameraDistance = 120.0f; // Дистанция камеры
float g_flDeathCameraHeight = 50.0f;    // Высота камеры
bool g_bDeathCameraInitialized = false; // Флаг инициализации камеры смерти

Vector g_vecDeathCameraAngles; // ★★★★ СОХРАНЕНИЕ УГЛОВ КАМЕРЫ СМЕРТИ ★★★★


Vector g_vecAssassinWishDir;
BOOL g_bAssassinWishJump = FALSE;
BOOL g_bAssassinWishAttack = FALSE;
float g_flAssassinWishYaw = 0;


BOOL g_bIsJumpCharging = FALSE;


// ★★★★ СИСТЕМА АВТОМАТИЧЕСКОГО ПОДЪЕМА ПО СТУПЕНЬКАМ ★★★★
float g_flStepHeight = 18.0f; // Максимальная высота ступеньки
float g_flStepCheckDistance = 24.0f; // Дистанция проверки ступенек
bool g_bStepLock = false; // Блокировка для избежания циклического подъема
float g_flLastStepTime = 0;
float g_flStepCooldown = 0.3f; // Защита от спама


// ★★★★ ДЕБАГ СИСТЕМА ОТРИСОВКИ BBOX ★★★★
bool g_bDrawAssassinBBox = false; // Начинаем с false
float g_flLastBBoxDraw = 0;
float g_flBBoxDrawInterval = 0.1f;

void DrawLine(float x1, float y1, float z1, float x2, float y2, float z2, int r, int g, int b);
void DebugDrawAssassinFullBBox(void);

bool Assassin_CheckStepUp(void);
void Assassin_PerformStepUp(const Vector& stepPos);

float g_flCurrentSurfaceZ = 0;
bool g_bSurfaceInitialized = false;



static Vector smoothPos = Vector(0, 0, 0);


bool g_bIsWalking = false;           // Флаг режима ходьбы
float g_flWalkSpeed = 150.0f;        // Скорость ходьбы
float g_flRunSpeed = 400.0f;         // Скорость бега (оригинальная)


float g_flAssassinWishPitch = 0;

BOOL g_bAssassinWishAltAttack = FALSE;


#define		ASSASSIN_AE_MELEE1	4
#define		ASSASSIN_AE_MELEE2	5






bool g_bWeaponsHidden = false;
int g_iOriginalWeaponsFlags = 0;
int g_iOriginalFOV = 0;





void HidePlayerWeapons(CBasePlayer* pPlayer)
{
	if (!pPlayer || g_bWeaponsHidden) return;

	ALERT(at_console, "=== HIDING PLAYER WEAPONS ===\n");

	// Сохраняем оригинальные флаги оружия
	g_iOriginalWeaponsFlags = pPlayer->pev->weapons;
	g_iOriginalFOV = pPlayer->m_iFOV;

	// Скрываем визуальные модели оружия
	pPlayer->pev->viewmodel = 0;
	pPlayer->pev->weaponmodel = 0;

	// Отключаем возможность атаки на очень большое время
	pPlayer->m_flNextAttack = 1e30;

	// Сбрасываем FOV от оружия
	pPlayer->m_iFOV = 0;
	pPlayer->m_iClientFOV = 0;

	// Если есть активное оружие, убираем его с экрана
	if (pPlayer->m_pActiveItem)
	{
		CBasePlayerWeapon* pWeapon = (CBasePlayerWeapon*)pPlayer->m_pActiveItem->GetWeaponPtr();
		if (pWeapon)
		{
			pWeapon->Holster();
		}
	}

	g_bWeaponsHidden = true;

	ALERT(at_console, "  Weapons hidden, original flags: 0x%08X\n", g_iOriginalWeaponsFlags);
}






void ShowPlayerWeapons(CBasePlayer* pPlayer)
{
	if (!pPlayer || !g_bWeaponsHidden) return;

	ALERT(at_console, "=== SHOWING PLAYER WEAPONS ===\n");

	// Восстанавливаем флаги оружия
	pPlayer->pev->weapons = g_iOriginalWeaponsFlags;

	// Восстанавливаем FOV
	pPlayer->m_iFOV = g_iOriginalFOV;

	// Сбрасываем время атаки, чтобы игрок мог стрелять
	pPlayer->m_flNextAttack = 0;

	// Восстанавливаем активное оружие
	if (pPlayer->m_pActiveItem)
	{
		CBasePlayerWeapon* pWeapon = (CBasePlayerWeapon*)pPlayer->m_pActiveItem->GetWeaponPtr();
		if (pWeapon)
		{
			pWeapon->Deploy();
		}
	}

	// Обновляем визуальные модели оружия для клиента
	if (pPlayer->m_pActiveItem)
	{
		pPlayer->m_pActiveItem->UpdateClientData(pPlayer);
	}

	// Сбрасываем сохранённые значения
	g_bWeaponsHidden = false;
	g_iOriginalWeaponsFlags = 0;
	g_iOriginalFOV = 0;

	ALERT(at_console, "  Weapons shown\n");
}






bool g_bOriginalDuckState = false;
float g_flOriginalDuckTime = 0.0f;
bool g_bDuckStateSaved = false;






void SavePlayerDuckState(CBasePlayer* pPlayer)
{
	if (!pPlayer || g_bDuckStateSaved) return;

	// Сохраняем текущее состояние приседания
	g_bOriginalDuckState = (pPlayer->pev->bInDuck != 0);
	g_flOriginalDuckTime = pPlayer->pev->flDuckTime;
	g_bDuckStateSaved = true;

	ALERT(at_console, "  Duck state saved: bInDuck=%d, flDuckTime=%.2f\n",
		g_bOriginalDuckState, g_flOriginalDuckTime);
}








void RestorePlayerDuckState(CBasePlayer* pPlayer)
{
	if (!pPlayer || !g_bDuckStateSaved) return;

	// Восстанавливаем состояние приседания
	pPlayer->pev->bInDuck = g_bOriginalDuckState;
	pPlayer->pev->flDuckTime = g_flOriginalDuckTime;

	// Если игрок был в приседе, применяем соответствующие параметры
	if (g_bOriginalDuckState)
	{
		pPlayer->pev->flags |= FL_DUCKING;
		pPlayer->pev->view_ofs = VEC_DUCK_VIEW;
		pPlayer->pev->mins = VEC_DUCK_HULL_MIN;
		pPlayer->pev->maxs = VEC_DUCK_HULL_MAX;
	}
	else
	{
		pPlayer->pev->flags &= ~FL_DUCKING;
		pPlayer->pev->view_ofs = VEC_VIEW;
		pPlayer->pev->mins = VEC_HUMAN_HULL_MIN;
		pPlayer->pev->maxs = VEC_HUMAN_HULL_MAX;
	}

	g_bDuckStateSaved = false;

	ALERT(at_console, "  Duck state restored: bInDuck=%d\n", g_bOriginalDuckState);
}




void DisablePlayerDuck(CBasePlayer* pPlayer)
{
	if (!pPlayer) return;

	// Принудительно поднимаем игрока из приседа
	pPlayer->pev->bInDuck = 0;
	pPlayer->pev->flDuckTime = 0;
	pPlayer->pev->flags &= ~FL_DUCKING;

	// Возвращаем стандартные размеры и высоту обзора
	pPlayer->pev->view_ofs = VEC_VIEW;
	pPlayer->pev->mins = VEC_HUMAN_HULL_MIN;
	pPlayer->pev->maxs = VEC_HUMAN_HULL_MAX;
}


bool g_bLastInvPressed = false;
bool g_bReloadPressed = false;
bool g_bInvisible = false;
float g_flLastGrenadeTime = 0;
float g_flGrenadeCooldown = 5.0f;        // Кулдаун между гранатами
float g_flLastInvisToggleTime = 0;
float g_flInvisToggleCooldown = 5.0f;    // Кулдаун между переключениями невидимости (5 сек)
float g_flNextInvisToggleTime = 0;       // Время, когда следующее переключение будет доступно

bool g_bInvisTransitioning = false;      // Флаг процесса перехода
float g_flInvisTargetRenderamt = 20;     // Целевая прозрачность при невидимости
float g_flInvisTransitionSpeed = 50.0f;  // Скорость перехода (как у оригинала)














void DrawLine(float x1, float y1, float z1, float x2, float y2, float z2, int r, int g, int b)
{
	MESSAGE_BEGIN(MSG_BROADCAST, SVC_TEMPENTITY);
	WRITE_BYTE(TE_BEAMPOINTS);
	WRITE_COORD(x1);
	WRITE_COORD(y1);
	WRITE_COORD(z1);
	WRITE_COORD(x2);
	WRITE_COORD(y2);
	WRITE_COORD(z2);
	WRITE_SHORT(g_sModelIndexLaser);
	WRITE_BYTE(0);        // start frame
	WRITE_BYTE(0);        // frame rate
	WRITE_BYTE(10);       // life in 0.1's (1 секунда) - короче для мигания
	WRITE_BYTE(5);        // line width in 0.1's
	WRITE_BYTE(0);        // noise amplitude in 0.01's
	WRITE_BYTE(r);        // red
	WRITE_BYTE(g);        // green
	WRITE_BYTE(b);        // blue
	WRITE_BYTE(255);      // brightness
	WRITE_BYTE(0);        // scroll speed in 0.1's
	MESSAGE_END();
}


Vector CalculateMoveDirection(void)
{
	Vector moveDirection = Vector(0, 0, 0);

	// Приоритет направлений (как в оригинальных играх)
	if (g_bMoveForward)
		moveDirection = moveDirection + Vector(1, 0, 0);
	if (g_bMoveBackward)
		moveDirection = moveDirection + Vector(-1, 0, 0);
	if (g_bMoveRight)
		moveDirection = moveDirection + Vector(0, 1, 0);
	if (g_bMoveLeft)
		moveDirection = moveDirection + Vector(0, -1, 0);

	// Нормализуем если двигаемся в нескольких направлениях
	if (moveDirection.Length() > 0)
		moveDirection = moveDirection.Normalize();

	return moveDirection;
}


void DebugDrawTraceLines(CBasePlayer* pPlayer) {
	if (!g_pAssassin || !g_bIsActive) return;

	Vector mouseAngles = pPlayer->pev->v_angle;
	Vector assassinPos = g_pAssassin->pev->origin;
	Vector cameraPos = pPlayer->pev->origin;

	Vector forward, right, up;
	g_engfuncs.pfnAngleVectors(mouseAngles, forward, right, up);

	// Параметры камеры
	float maxDistance = 120.0f;
	float minDistance = 30.0f;
	float maxHeight = 40.0f;
	float minHeight = 10.0f;
	float baseHeight = 25.0f;

	float verticalAngle = mouseAngles.x;

	// Вычисляем желаемую позицию камеры
	float desiredDistance = maxDistance;
	if (verticalAngle < 0.0f) {
		float angleRatio = fabs(verticalAngle) / 89.0f;
		desiredDistance = maxDistance - (maxDistance - minDistance) * angleRatio;
	}

	float desiredHeight = baseHeight;
	if (verticalAngle > 0.0f) {
		float angleRatio = verticalAngle / 89.0f;
		desiredHeight = baseHeight + (maxHeight - baseHeight) * angleRatio;
	}
	else if (verticalAngle < 0.0f) {
		float angleRatio = fabs(verticalAngle) / 89.0f;
		desiredHeight = baseHeight - (baseHeight - minHeight) * angleRatio;
	}

	Vector desiredCameraPos = assassinPos + (-forward * desiredDistance) + (up * desiredHeight);

	// Точки для трассировки
	float assassinHeadHeight = 64.0f;
	Vector assassinHeadPos = assassinPos + Vector(0, 0, assassinHeadHeight);

	float cameraHeadHeight = 28.0f; // Высота головы камеры
	Vector cameraHeadPos = cameraPos + Vector(0, 0, cameraHeadHeight);
	Vector desiredHeadPos = desiredCameraPos + Vector(0, 0, cameraHeadHeight);

	// 1. РЕАЛЬНАЯ ТРАССИРОВКА (от головы ассасина к голове камеры)
	TraceResult trReal;
	UTIL_TraceHull(
		assassinHeadPos,
		desiredHeadPos,
		ignore_monsters,
		point_hull,
		pPlayer->edict(),
		&trReal
	);

	Vector realHitPos = (trReal.flFraction < 1.0f) ? trReal.vecEndPos : desiredHeadPos;

	// 2. Отрисовываем белый луч (реальная трассировка к голове камеры)
	MESSAGE_BEGIN(MSG_BROADCAST, SVC_TEMPENTITY);
	WRITE_BYTE(TE_BEAMPOINTS);
	WRITE_COORD(assassinHeadPos.x);
	WRITE_COORD(assassinHeadPos.y);
	WRITE_COORD(assassinHeadPos.z);
	WRITE_COORD(realHitPos.x);
	WRITE_COORD(realHitPos.y);
	WRITE_COORD(realHitPos.z);
	WRITE_SHORT(g_sModelIndexLaser);
	WRITE_BYTE(0);
	WRITE_BYTE(10);
	WRITE_BYTE(1);
	WRITE_BYTE(5);
	WRITE_BYTE(0);
	WRITE_BYTE(255);  // R
	WRITE_BYTE(255);  // G
	WRITE_BYTE(255);  // B
	WRITE_BYTE(255);  // Яркость
	WRITE_BYTE(0);
	MESSAGE_END();

	// 3. Отрисовываем зеленый луч (идеальная траектория к голове камеры)
	MESSAGE_BEGIN(MSG_BROADCAST, SVC_TEMPENTITY);
	WRITE_BYTE(TE_BEAMPOINTS);
	WRITE_COORD(assassinHeadPos.x);
	WRITE_COORD(assassinHeadPos.y);
	WRITE_COORD(assassinHeadPos.z);
	WRITE_COORD(desiredHeadPos.x);
	WRITE_COORD(desiredHeadPos.y);
	WRITE_COORD(desiredHeadPos.z);
	WRITE_SHORT(g_sModelIndexLaser);
	WRITE_BYTE(0);
	WRITE_BYTE(10);
	WRITE_BYTE(1);
	WRITE_BYTE(3);
	WRITE_BYTE(0);
	WRITE_BYTE(0);    // R
	WRITE_BYTE(255);  // G
	WRITE_BYTE(0);    // B
	WRITE_BYTE(128);  // Яркость
	WRITE_BYTE(0);
	MESSAGE_END();

	// 4. Отрисовываем красный маркер текущей головы камеры
	float headMarkerSize = 2.0f;
	MESSAGE_BEGIN(MSG_BROADCAST, SVC_TEMPENTITY);
	WRITE_BYTE(TE_BEAMPOINTS);
	WRITE_COORD(cameraHeadPos.x - headMarkerSize);
	WRITE_COORD(cameraHeadPos.y);
	WRITE_COORD(cameraHeadPos.z);
	WRITE_COORD(cameraHeadPos.x + headMarkerSize);
	WRITE_COORD(cameraHeadPos.y);
	WRITE_COORD(cameraHeadPos.z);
	WRITE_SHORT(g_sModelIndexLaser);
	WRITE_BYTE(0);
	WRITE_BYTE(10);
	WRITE_BYTE(1);
	WRITE_BYTE(2);
	WRITE_BYTE(0);
	WRITE_BYTE(255);  // R
	WRITE_BYTE(0);    // G
	WRITE_BYTE(0);    // B
	WRITE_BYTE(255);  // Яркость
	WRITE_BYTE(0);
	MESSAGE_END();

	// 5. Показываем точку столкновения (если есть)
	if (trReal.flFraction < 1.0f) {
		// Желтая звезда в точке столкновения
		float markerSize = 3.0f;

		MESSAGE_BEGIN(MSG_BROADCAST, SVC_TEMPENTITY);
		WRITE_BYTE(TE_BEAMPOINTS);
		WRITE_COORD(realHitPos.x - markerSize);
		WRITE_COORD(realHitPos.y);
		WRITE_COORD(realHitPos.z);
		WRITE_COORD(realHitPos.x + markerSize);
		WRITE_COORD(realHitPos.y);
		WRITE_COORD(realHitPos.z);
		WRITE_SHORT(g_sModelIndexLaser);
		WRITE_BYTE(0);
		WRITE_BYTE(10);
		WRITE_BYTE(1);
		WRITE_BYTE(3);
		WRITE_BYTE(0);
		WRITE_BYTE(255);  // R
		WRITE_BYTE(255);  // G
		WRITE_BYTE(0);    // B
		WRITE_BYTE(255);  // Яркость
		WRITE_BYTE(0);
		MESSAGE_END();
	}

	// 6. Выводим информацию
	ALERT(at_console, "Ray to camera head - Fraction: %.3f\n", trReal.flFraction);
	ALERT(at_console, "Assassin head: Z=%.1f, Camera head: Z=%.1f\n",
		assassinHeadPos.z, cameraHeadPos.z);
}


//=========================================================
// monster-specific schedule types
//=========================================================
enum
{
	SCHED_ASSASSIN_EXPOSED = LAST_COMMON_SCHEDULE + 1,// cover was blown.
	SCHED_ASSASSIN_JUMP,	// fly through the air
	SCHED_ASSASSIN_JUMP_ATTACK,	// fly through the air and shoot
	SCHED_ASSASSIN_JUMP_LAND, // hit and run away
};

//=========================================================
// monster-specific tasks
//=========================================================

enum
{
	TASK_ASSASSIN_FALL_TO_GROUND = LAST_COMMON_TASK + 1, // falling and waiting to hit ground
};


//=========================================================
// Monster's Anim Events Go Here
//=========================================================
#define		ASSASSIN_AE_SHOOT1	1
#define		ASSASSIN_AE_TOSS1	2
#define		ASSASSIN_AE_JUMP	3


#define bits_MEMORY_BADJUMP		(bits_MEMORY_CUSTOM1)






















class CHAssassin : public CBaseMonster
{
public:
	void Spawn( void );
	void Precache( void );
	void SetYawSpeed ( void );


	// Добавляем метод для проверки управления
	BOOL IsBeingControlled(void) { return g_bIsActive && g_pAssassin == this; }
	// Модифицируем Classify
	int  Classify ( void ) override;


		int  ISoundMask ( void);
	void Shoot( void );
	void HandleAnimEvent( MonsterEvent_t *pEvent );
	Schedule_t* GetSchedule ( void );
	Schedule_t* GetScheduleOfType ( int Type );
	BOOL CheckMeleeAttack1 ( float flDot, float flDist );	// jump
	// BOOL CheckMeleeAttack2 ( float flDot, float flDist );
	BOOL CheckRangeAttack1 ( float flDot, float flDist );	// shoot
	BOOL CheckRangeAttack2 ( float flDot, float flDist );	// throw grenade
	void StartTask ( Task_t *pTask );
	void RunAI( void );
	void RunTask ( Task_t *pTask );
	void DeathSound ( void );
	void IdleSound ( void );
	CUSTOM_SCHEDULES;

	int	Save( CSave &save ); 
	int Restore( CRestore &restore );
	static TYPEDESCRIPTION m_SaveData[];

	float m_flLastShot;
	float m_flDiviation;

	float m_flNextJump;
	Vector m_vecJumpVelocity;

	float m_flNextGrenadeCheck;
	Vector	m_vecTossVelocity;
	BOOL	m_fThrowGrenade;

	int		m_iTargetRanderamt;

	int		m_iFrustration;

	int		m_iShell;




	float m_flPlayerWishPitch;

	BOOL m_bPlayerWishAltAttack;

	int m_iLastAltAttackType;



	void SetPlayerWishAltAttack(BOOL altAttack) {m_bPlayerWishAltAttack = altAttack;}

	void PerformAltAttack(void);

	void PerformMeleeAttack(void);
	float m_flNextMeleeAttack;  // Кулдаун ближней атаки

	void SetPlayerWishPitch(float pitch) { m_flPlayerWishPitch = pitch; }

	void PlayerControlledDeath(void);


	// ★★★★ МЕТОДЫ ДЛЯ РАБОТЫ С СИСТЕМОЙ ВВОДА ★★★★
	void UpdatePlayerControl(void);
	BOOL ShouldUsePlayerInput(void) { return IsBeingControlled() && HasPlayerInput(); }
	BOOL HasPlayerInput(void) {
		return (g_vecAssassinWishDir.Length() > 0.1f || g_bAssassinWishJump || g_bAssassinWishAttack);
	}


	// Новые методы для управления от игрока
	void SetPlayerWishDir(const Vector& dir) { m_vecPlayerWishDir = dir; }
	void SetPlayerWishYaw(float yaw) { m_flPlayerWishYaw = yaw; }
	void SetPlayerWishJump(BOOL jump) { m_bPlayerWishJump = jump; }
	void SetPlayerWishAttack(BOOL attack) { m_bPlayerWishAttack = attack; }

	// Флаг, что игрок хочет двигаться
	BOOL PlayerWantsToMove() { return m_vecPlayerWishDir.Length() > 0.1f; }

	// Обновление AI на основе команд игрока
	void UpdateAIMovement(void);


	void MovementComplete(void);



	// ★★★★ НОВЫЕ МЕТОДЫ ДЛЯ ХОДЬБЫ ★★★★
	void SetPlayerWalking(BOOL walking) { m_bPlayerWalking = walking; }
	BOOL IsWalking(void) { return m_bPlayerWalking; }
	float GetCurrentSpeed(void) { return m_bPlayerWalking ? g_flWalkSpeed : g_flRunSpeed; }


	BOOL IsSliding(void) { return m_bIsSliding; }

	Activity GetCurrentActivity(void)
	{
		if (m_bPlayerWalking)
			return ACT_WALK;
		else
			return ACT_RUN;
	}


private:
	float m_flLastPlayerInputTime;
	float m_flNextDebugTime;

	// Переменные для хранения команд игрока
	Vector m_vecPlayerWishDir;
	float m_flPlayerWishYaw;
	BOOL m_bPlayerWishJump;
	BOOL m_bPlayerWishAttack;

	// Для отслеживания состояния
	Vector m_vecLastAIGoal;
	float m_flLastAIMoveTime;
	BOOL m_bAIMoving;
	
	BOOL m_bPlayerWalking;
	BOOL m_bIsSliding;

};
LINK_ENTITY_TO_CLASS( monster_human_assassin, CHAssassin );





























// ============================================
// БЛОК 2: ИСПРАВЛЕННАЯ ФУНКЦИЯ НЕВИДИМОСТИ
// ============================================
// Разместить в hassassin.cpp

void Assassin_ActivateInvisibility(void)
{
	if (!g_pAssassin || !g_bIsActive)
	{
		ALERT(at_console, "Invisibility: assassin not active\n");
		return;
	}

	if (gpGlobals->time < g_flNextInvisToggleTime)
	{
		float remaining = g_flNextInvisToggleTime - gpGlobals->time;
		ALERT(at_console, "Invisibility: on cooldown (%.1f sec remaining)\n", remaining);
		return;
	}

	CHAssassin* pAssassin = (CHAssassin*)g_pAssassin;

	if (pAssassin->pev->deadflag != DEAD_NO || (pAssassin->pev->flags & FL_KILLME))
	{
		ALERT(at_console, "Invisibility: assassin is dead or dying\n");
		return;
	}

	if (g_bInvisible)
	{
		// Выход из невидимости — возвращаемся к обычному поведению
		ALERT(at_console, ">>> DEACTIVATING invisibility <<<\n");

		g_bInvisible = false;
		g_bInvisTransitioning = false;

		// Устанавливаем целевое значение как у оригинала (зависит от состояния)
		if (g_iSkillLevel != SKILL_HARD || pAssassin->m_hEnemy == NULL ||
			pAssassin->pev->deadflag != DEAD_NO ||
			pAssassin->m_Activity == ACT_RUN ||
			pAssassin->m_Activity == ACT_WALK ||
			!(pAssassin->pev->flags & FL_ONGROUND))
		{
			pAssassin->m_iTargetRanderamt = 255;
		}
		else
		{
			pAssassin->m_iTargetRanderamt = 20;
		}

		EMIT_SOUND(pAssassin->edict(), CHAN_BODY, "debris/beamstart1.wav", 0.5, ATTN_NORM);
	}
	else
	{
		// Вход в невидимость — запускаем плавный переход
		ALERT(at_console, ">>> ACTIVATING invisibility (smooth transition) <<<\n");

		g_bInvisible = true;
		g_bInvisTransitioning = true;
		g_flInvisTargetRenderamt = 5;

		EMIT_SOUND(pAssassin->edict(), CHAN_BODY, "debris/beamstart1.wav", 0.5, ATTN_NORM);
	}

	g_flNextInvisToggleTime = gpGlobals->time + g_flInvisToggleCooldown;
	g_flLastInvisToggleTime = gpGlobals->time;

	ALERT(at_console, "Invisibility: %s\n", g_bInvisible ? "ON" : "OFF");
}








// ============================================
// БЛОК 3: ИСПРАВЛЕННАЯ UpdateInvisibility С ДИАГНОСТИКОЙ
// ============================================
// Разместить в hassassin.cpp

void Assassin_UpdateInvisibility(void)
{
	if (!g_pAssassin || !g_bIsActive || !g_bInvisible) return;

	CHAssassin* pAssassin = (CHAssassin*)g_pAssassin;

	// ★★★ ПРОВЕРЯЕМ, НЕ УСТАНАВЛИВАЕТ ЛИ КТО-ТО renderamt В 0 ★★★
	if (pAssassin->pev->renderamt == 0)
	{
		ALERT(at_console, "WARNING: renderamt was 0! Resetting to 20\n");
		pAssassin->pev->renderamt = 20;
		pAssassin->pev->rendermode = kRenderTransTexture;
	}

	// Определяем целевую прозрачность
	float targetRenderamt;

	if (pAssassin->m_Activity == ACT_RUN || pAssassin->m_Activity == ACT_WALK)
	{
		targetRenderamt = 20;   // Движется
	}
	else if (!(pAssassin->pev->flags & FL_ONGROUND))
	{
		targetRenderamt = 20;   // В воздухе
	}
	else
	{
		targetRenderamt = 5;    // Стоит на месте
	}

	// Плавный переход
	if (pAssassin->pev->renderamt > targetRenderamt)
	{
		pAssassin->pev->renderamt = max(
			pAssassin->pev->renderamt - g_flInvisTransitionSpeed,
			targetRenderamt
		);
		pAssassin->pev->rendermode = kRenderTransTexture;
		pAssassin->pev->renderfx = kRenderFxGlowShell;
	}
	else if (pAssassin->pev->renderamt < targetRenderamt)
	{
		pAssassin->pev->renderamt = min(
			pAssassin->pev->renderamt + g_flInvisTransitionSpeed,
			targetRenderamt
		);
		pAssassin->pev->rendermode = kRenderTransTexture;
		pAssassin->pev->renderfx = kRenderFxGlowShell;
	}

	static float lastDebugTime = 0;
	if (gpGlobals->time - lastDebugTime > 0.5f)
	{
		ALERT(at_console, "Invis: renderamt=%d, target=%.0f, activity=%d, onground=%d\n",
			pAssassin->pev->renderamt, targetRenderamt,
			pAssassin->m_Activity,
			(pAssassin->pev->flags & FL_ONGROUND) ? 1 : 0);
		lastDebugTime = gpGlobals->time;
	}
}














void Assassin_ThrowGrenade(void)
{
	if (!g_pAssassin || !g_bIsActive) return;

	CHAssassin* pAssassin = (CHAssassin*)g_pAssassin;

	// Проверка кулдауна
	if (gpGlobals->time - g_flLastGrenadeTime < g_flGrenadeCooldown)
	{
		ALERT(at_console, "Grenade on cooldown (%.1f sec remaining)\n",
			g_flGrenadeCooldown - (gpGlobals->time - g_flLastGrenadeTime));
		return;
	}

	ALERT(at_console, "Assassin throwing grenade!\n");

	// Используем углы ассасина для направления броска
	UTIL_MakeVectors(pAssassin->pev->angles);

	// Позиция броска - от груди ассасина
	Vector vecSrc = pAssassin->pev->origin + Vector(0, 0, 48);

	// Направление и сила броска
	Vector vecThrow = gpGlobals->v_forward * 600 + gpGlobals->v_up * 200;

	// ★★★ СОЗДАЁМ ТОЛЬКО ОДНУ ГРАНАТУ ★★★
	CGrenade::ShootTimed(pAssassin->pev, vecSrc, vecThrow, 3.0);

	// Звук броска
	EMIT_SOUND(pAssassin->edict(), CHAN_WEAPON, "weapons/grenade1.wav", 0.8, ATTN_NORM);

	// Анимация броска
	int tossSequence = pAssassin->LookupSequence("toss");
	if (tossSequence == -1)
		tossSequence = pAssassin->LookupActivity(ACT_RANGE_ATTACK2);

	if (tossSequence != -1)
	{
		pAssassin->pev->sequence = tossSequence;
		pAssassin->pev->frame = 0;
		pAssassin->ResetSequenceInfo();
	}

	g_flLastGrenadeTime = gpGlobals->time;

	ALERT(at_console, "Grenade thrown! Next grenade available at: %.2f\n",
		g_flLastGrenadeTime + g_flGrenadeCooldown);
}











void Assassin_PlayerTriggeredJump(void)
{
	if (!g_pAssassin)
		return;

	if (g_pAssassin->pev->health <= 0)
	{
		ALERT(at_console, "Assassin is dead - cannot jump\n");
		g_bJumpCharging = false;
		g_flJumpChargeAmount = 0;
		return;
	}

	CHAssassin* pAssassin = (CHAssassin*)g_pAssassin;

	// Проверка кулдауна
	if (pAssassin->m_flNextJump > gpGlobals->time)
	{
		g_bJumpCharging = false;
		g_flJumpChargeAmount = 0;
		return;
	}

	// Проверка что на земле
	if (!(pAssassin->pev->flags & FL_ONGROUND))
	{
		g_bJumpCharging = false;
		g_flJumpChargeAmount = 0;
		return;
	}

	float flGravity = g_psv_gravity->value;

	// ★★★★ СОХРАНЯЕМ ГОРИЗОНТАЛЬНУЮ СКОРОСТЬ ★★★★
	float savedVelocityX = pAssassin->pev->velocity.x;
	float savedVelocityY = pAssassin->pev->velocity.y;

	// ★★★★ РАСЧЁТ СКОРОСТИ ПРЫЖКА НА ОСНОВЕ ЗАРЯДА ★★★★
	float charge = g_flJumpChargeAmount;  // 0.0 - 1.0

	// Минимальный заряд (даже при быстром нажатии)
	if (charge < 0.15f)
		charge = 0.15f;

	// Линейная интерполяция между мин и макс скоростью
	float verticalSpeed = g_flMinJumpVelocity + (g_flMaxJumpVelocity - g_flMinJumpVelocity) * charge;

	// Расчёт высоты прыжка (для информации)
	float jumpHeight = (verticalSpeed * verticalSpeed) / (2.0f * flGravity);

	ALERT(at_console, "JUMP: charge=%.2f, verticalSpeed=%.0f, height=%.0f\n",
		charge, verticalSpeed, jumpHeight);

	// ★★★★ УСТАНАВЛИВАЕМ ФИЗИКУ ПРЫЖКА ★★★★
	pAssassin->pev->movetype = MOVETYPE_TOSS;
	pAssassin->pev->flags &= ~FL_ONGROUND;

	// Сохраняем горизонтальную скорость
	pAssassin->pev->velocity.x = savedVelocityX;
	pAssassin->pev->velocity.y = savedVelocityY;
	pAssassin->pev->velocity.z = verticalSpeed;

	// Анимация прыжка
	pAssassin->SetActivity(ACT_HOP);

	// Кулдаун
	pAssassin->m_flNextJump = gpGlobals->time + g_flJumpCooldown;

	// Звук прыжка
	switch (RANDOM_LONG(0, 3))
	{
	case 0: EMIT_SOUND(pAssassin->edict(), CHAN_BODY, "player/pl_step1.wav", 0.7, ATTN_NORM); break;
	case 1: EMIT_SOUND(pAssassin->edict(), CHAN_BODY, "player/pl_step2.wav", 0.7, ATTN_NORM); break;
	case 2: EMIT_SOUND(pAssassin->edict(), CHAN_BODY, "player/pl_step3.wav", 0.7, ATTN_NORM); break;
	case 3: EMIT_SOUND(pAssassin->edict(), CHAN_BODY, "player/pl_step4.wav", 0.7, ATTN_NORM); break;
	}

	// Сброс заряда
	g_bJumpCharging = false;
	g_flJumpChargeAmount = 0;

	ALERT(at_console, "Jump executed! Next jump at: %.2f\n", pAssassin->m_flNextJump);
}







void CHAssassin::PerformMeleeAttack(void)
{
	if (!IsBeingControlled())
		return;

	// ★★★★ ПРОВЕРКА КУЛДАУНА ★★★★
	if (m_flNextMeleeAttack > gpGlobals->time)
		return;

	// ★★★★ СЛУЧАЙНЫЙ ВЫБОР ТИПА АТАКИ ★★★★
	m_iLastAltAttackType = RANDOM_LONG(0, 1);

	ALERT(at_console, "PerformMeleeAttack type %d\n", m_iLastAltAttackType);

	int chosenSequence = -1;
	Activity chosenActivity;
	float flDamage;
	float flRange;
	float flCooldown;
	const char* attackSound;

	if (m_iLastAltAttackType == 0)
	{
		// ★★★★ БЫСТРАЯ АТАКА ★★★★
		chosenSequence = LookupSequence("melee1");
		if (chosenSequence == -1)
			chosenSequence = LookupActivity(ACT_MELEE_ATTACK1);

		chosenActivity = ACT_MELEE_ATTACK1;
		flDamage = 10.0f;
		flRange = 64.0f;
		flCooldown = 0.6f;
		attackSound = "zombie/claw_miss1.wav";

		ALERT(at_console, "Fast melee attack: dmg=%.0f, range=%.0f, cooldown=%.1f\n",
			flDamage, flRange, flCooldown);
	}
	else
	{
		// ★★★★ ТЯЖЁЛАЯ АТАКА ★★★★
		chosenSequence = LookupSequence("melee2");
		if (chosenSequence == -1)
			chosenSequence = LookupActivity(ACT_MELEE_ATTACK2);

		chosenActivity = ACT_MELEE_ATTACK2;
		flDamage = 25.0f;
		flRange = 80.0f;
		flCooldown = 1.0f;
		attackSound = "zombie/claw_miss2.wav";

		ALERT(at_console, "Heavy melee attack: dmg=%.0f, range=%.0f, cooldown=%.1f\n",
			flDamage, flRange, flCooldown);
	}

	// ★★★★ ПРОВЕРКА НАЛИЧИЯ АНИМАЦИИ ★★★★
	if (chosenSequence == -1)
	{
		ALERT(at_console, "ERROR: No melee sequence found!\n");
		return;
	}

	// ★★★★ УСТАНОВКА АНИМАЦИИ ★★★★
	pev->sequence = chosenSequence;
	pev->frame = 0;
	ResetSequenceInfo();
	SetActivity(chosenActivity);

	ALERT(at_console, "Melee sequence: %d, activity: %d\n",
		chosenSequence, (int)chosenActivity);

	// ★★★★ ЗВУК ЗАМАХА ★★★★
	EMIT_SOUND(ENT(pev), CHAN_BODY, attackSound, 1.0, ATTN_NORM);

	// ★★★★ ТРАССИРОВКА УДАРА ★★★★
	Vector forward;
	UTIL_MakeVectors(pev->angles);
	forward = gpGlobals->v_forward;

	Vector vecSrc = pev->origin + Vector(0, 0, 36);  // Центр тела
	Vector vecEnd = vecSrc + forward * flRange;

	TraceResult tr;
	UTIL_TraceHull(vecSrc, vecEnd, dont_ignore_monsters, head_hull, ENT(pev), &tr);

	if (tr.pHit)
	{
		CBaseEntity* pHitEntity = CBaseEntity::Instance(tr.pHit);
		if (pHitEntity && pHitEntity != this && pHitEntity->pev->takedamage)
		{
			// ★★★★ ЗВУК ПОПАДАНИЯ ★★★★
			switch (RANDOM_LONG(0, 2))
			{
			case 0: EMIT_SOUND(ENT(pev), CHAN_BODY, "weapons/cbar_hitbod1.wav", 1.0, ATTN_NORM); break;
			case 1: EMIT_SOUND(ENT(pev), CHAN_BODY, "weapons/cbar_hitbod2.wav", 1.0, ATTN_NORM); break;
			case 2: EMIT_SOUND(ENT(pev), CHAN_BODY, "weapons/cbar_hitbod3.wav", 1.0, ATTN_NORM); break;
			}

			// ★★★★ НАНЕСЕНИЕ УРОНА ★★★★
			pHitEntity->TakeDamage(pev, pev, flDamage, DMG_CLUB);

			ALERT(at_console, "Melee hit: %s for %.0f damage\n",
				STRING(pHitEntity->pev->classname), flDamage);

			// ★★★★ ОТБРАСЫВАНИЕ ★★★★
			if (pHitEntity->pev->flags & FL_ONGROUND)
			{
				Vector pushDir = (pHitEntity->pev->origin - pev->origin).Normalize();
				pHitEntity->pev->velocity = pHitEntity->pev->velocity + pushDir * flDamage * 10;
			}
		}
	}
	else
	{
		ALERT(at_console, "Melee miss\n");
	}

	// ★★★★ УСТАНОВКА КУЛДАУНА (ГАРАНТИРОВАННАЯ ЗАДЕРЖКА) ★★★★
	m_flNextMeleeAttack = gpGlobals->time + flCooldown;

	ALERT(at_console, "Next melee attack available at: %.2f (cooldown: %.1f)\n",
		m_flNextMeleeAttack, flCooldown);
}







void CHAssassin::PerformAltAttack(void)
{
	if (!IsBeingControlled())
		return;

	// Проверяем, что не находимся в другой анимации атаки
	if (m_Activity == ACT_MELEE_ATTACK2 || m_Activity == ACT_RANGE_ATTACK2)
	{
		if (!m_fSequenceFinished)
			return;  // Ждем завершения текущей атаки
	}

	ALERT(at_console, "Performing alt attack!\n");

	// ★★★★ СЛУЧАЙНЫЙ ВЫБОР МЕЖДУ ДВУМЯ АНИМАЦИЯМИ ★★★★
	int altAttackType = RANDOM_LONG(0, 1);

	if (altAttackType == 0)
	{
		// Первый вариант альтернативной атаки
		int seq1 = LookupSequence("alt_attack1");
		if (seq1 == -1)
			seq1 = LookupSequence("melee2");  // Fallback

		if (seq1 != -1)
		{
			pev->sequence = seq1;
			pev->frame = 0;
			ResetSequenceInfo();
			m_Activity = ACT_MELEE_ATTACK2;  // Используем melee2 как тип активности

			ALERT(at_console, "Alt attack animation 1 (seq: %d)\n", seq1);
		}
	}
	else
	{
		// Второй вариант альтернативной атаки
		int seq2 = LookupSequence("alt_attack2");
		if (seq2 == -1)
			seq2 = LookupSequence("range_attack2");  // Fallback

		if (seq2 != -1)
		{
			pev->sequence = seq2;
			pev->frame = 0;
			ResetSequenceInfo();
			m_Activity = ACT_RANGE_ATTACK2;  // Используем range2 как тип активности

			ALERT(at_console, "Alt attack animation 2 (seq: %d)\n", seq2);
		}
	}

	// Звук альтернативной атаки
	EMIT_SOUND(ENT(pev), CHAN_WEAPON, "weapons/pl_gun1.wav", RANDOM_FLOAT(0.8, 1.0), ATTN_NORM);

	m_bPlayerWishAltAttack = FALSE;
}




















void CHAssassin::MovementComplete(void)
{
	switch (m_iTaskStatus)
	{
		// ... другие case ...
	case TASKSTATUS_RUNNING_TASK:
		ALERT(at_error, "Movement completed twice!\n"); // ← закомментируйте эту строку
		break;
		// ...
	}
}




void DrawStepDebugLines(const Vector& start, const Vector& end, int r, int g, int b)
{
	MESSAGE_BEGIN(MSG_BROADCAST, SVC_TEMPENTITY);
	WRITE_BYTE(TE_BEAMPOINTS);
	WRITE_COORD(start.x);
	WRITE_COORD(start.y);
	WRITE_COORD(start.z);
	WRITE_COORD(end.x);
	WRITE_COORD(end.y);
	WRITE_COORD(end.z);
	WRITE_SHORT(g_sModelIndexLaser);
	WRITE_BYTE(0);
	WRITE_BYTE(0);
	WRITE_BYTE(10); // life
	WRITE_BYTE(5);  // width
	WRITE_BYTE(0);
	WRITE_BYTE(r);
	WRITE_BYTE(g);
	WRITE_BYTE(b);
	WRITE_BYTE(255);
	WRITE_BYTE(0);
	MESSAGE_END();
}
















void Assassin_SetPlayerInput(BOOL forward, BOOL back, BOOL left, BOOL right,
	BOOL jump, BOOL attack, BOOL altAttack, float yaw, float pitch)
{
	if (!g_pAssassin || !g_bIsActive) return;

	CHAssassin* pAssassin = (CHAssassin*)g_pAssassin;

	// ★★★ ПЕРЕИМЕНОВЫВАЕМ ЛОКАЛЬНЫЕ ПЕРЕМЕННЫЕ ★★★
	Vector vecForward, vecRight, vecUp;
	Vector angles(0, yaw, 0);
	UTIL_MakeVectors(angles);
	vecForward = gpGlobals->v_forward;
	vecRight = gpGlobals->v_right;
	vecUp = gpGlobals->v_up;

	// Теперь конфликта имён нет!
	Vector wishDir(0, 0, 0);
	if (forward) wishDir = wishDir + vecForward;
	if (back) wishDir = wishDir - vecForward;
	if (right) wishDir = wishDir + vecRight;
	if (left) wishDir = wishDir - vecRight;

	// Нормализуем
	if (wishDir.Length() > 0) {
		wishDir = wishDir.Normalize();
	}

	// ★★★ УЧИТЫВАЕМ PITCH ДЛЯ ВЕРТИКАЛЬНОГО ДВИЖЕНИЯ ★★★
	if (fabs(pitch) > 5.0f && wishDir.Length() > 0) {
		// Только в воздухе или на лестнице
		if (!(pAssassin->pev->flags & FL_ONGROUND) ||
			UTIL_PointContents(pAssassin->pev->origin + Vector(0, 0, 36)) == CONTENTS_LADDER)
		{
			float pitchRad = pitch * (M_PI / 180.0f);
			wishDir = wishDir * cos(pitchRad) + vecUp * sin(pitchRad);
			wishDir = wishDir.Normalize();
		}
	}

	// Сохраняем в ассасине
	pAssassin->SetPlayerWishDir(wishDir);
	pAssassin->SetPlayerWishYaw(yaw);
	pAssassin->SetPlayerWishPitch(pitch);
	pAssassin->SetPlayerWishJump(jump);
	pAssassin->SetPlayerWishAttack(attack);
	pAssassin->SetPlayerWishAltAttack(altAttack);
	pAssassin->SetPlayerWalking(g_bIsWalking);
}
















bool Assassin_CheckStepUp(void)
{
	if (!g_pAssassin || !g_bIsActive) return false;
	CHAssassin* pAssassin = (CHAssassin*)g_pAssassin;

	if (!(pAssassin->pev->flags & FL_ONGROUND)) return false;

	// Получаем направление движения
	UTIL_MakeVectors(pAssassin->pev->angles);
	Vector forward = gpGlobals->v_forward;
	Vector right = gpGlobals->v_right;
	Vector worldMoveDir = (forward * g_vecAssassinWishDir.x) + (right * g_vecAssassinWishDir.y);

	if (worldMoveDir.Length() < 0.1f) return false;
	worldMoveDir = worldMoveDir.Normalize();

	// ★★★★ ИНИЦИАЛИЗАЦИЯ ТЕКУЩЕЙ ВЫСОТЫ ПОВЕРХНОСТИ ★★★★
	if (!g_bSurfaceInitialized) {
		TraceResult tr;
		Vector groundEnd = pAssassin->pev->origin - Vector(0, 0, 64.0f);
		UTIL_TraceLine(pAssassin->pev->origin, groundEnd, ignore_monsters, pAssassin->edict(), &tr);
		g_flCurrentSurfaceZ = tr.vecEndPos.z;
		g_bSurfaceInitialized = true;
		ALERT(at_console, "Surface initialized: %.1f\n", g_flCurrentSurfaceZ);
	}

	// ★★★★ ПРОВЕРЯЕМ ВЫСОТУ ПОВЕРХНОСТИ ВПЕРЕДИ ★★★★
	Vector checkPoint = pAssassin->pev->origin + worldMoveDir * 8.0f;

	TraceResult trAhead;
	Vector groundEnd = checkPoint - Vector(0, 0, 64.0f);
	UTIL_TraceLine(checkPoint, groundEnd, ignore_monsters, pAssassin->edict(), &trAhead);

	float aheadSurfaceZ = trAhead.vecEndPos.z;
	float heightDiff = aheadSurfaceZ - g_flCurrentSurfaceZ;

	ALERT(at_console, "Surface check: Current=%.1f, Ahead=%.1f, Diff=%.1f\n",
		g_flCurrentSurfaceZ, aheadSurfaceZ, heightDiff);

	// ★★★★ ГИСТЕРЕЗИС: МЕНЯЕМ ВЫСОТУ ТОЛЬКО ПРИ ЗНАЧИТЕЛЬНОМ ИЗМЕНЕНИИ ★★★★
	if (fabs(heightDiff) > 4.0f && fabs(heightDiff) <= 18.0f) {
		// Обновляем текущую высоту поверхности
		g_flCurrentSurfaceZ = aheadSurfaceZ;

		// Устанавливаем новую высоту персонажа
		pAssassin->pev->origin.z = g_flCurrentSurfaceZ + 36.0f;

		ALERT(at_console, "=== SURFACE CHANGED: %.1f ===\n", heightDiff);
		return true;
	}

	return false;
}




















void CHAssassin::UpdatePlayerControl(void)
{
	if (!IsBeingControlled())
		return;

	pev->angles.y = m_flPlayerWishYaw;
	pev->v_angle.y = m_flPlayerWishYaw;

	// ★★★★ АВТОМАТИЧЕСКОЕ ВОССТАНОВЛЕНИЕ ПОСЛЕ ПРЫЖКА ★★★★
	if (pev->movetype == MOVETYPE_TOSS && (pev->flags & FL_ONGROUND))
	{
		pev->movetype = MOVETYPE_STEP;
		SetActivity(ACT_IDLE);
		ALERT(at_console, "Jump landed, physics restored\n");
	}

	// ★★★★ ПРОВЕРКА НАЖАТИЯ АТАКИ ★★★★
	BOOL bWantMelee = m_bPlayerWishAltAttack && (m_flNextMeleeAttack <= gpGlobals->time);
	BOOL bWantShoot = m_bPlayerWishAttack;

	// ★★★★ ПРОВЕРКА: ИДЁТ ЛИ АТАКА/ПРЫЖОК СЕЙЧАС? ★★★★
	BOOL bIsAttacking = (m_Activity == ACT_RANGE_ATTACK1 ||
		m_Activity == ACT_MELEE_ATTACK1 ||
		m_Activity == ACT_MELEE_ATTACK2);

	BOOL bIsJumping = (pev->movetype == MOVETYPE_TOSS && !(pev->flags & FL_ONGROUND));

	// ★★★★ ЕСЛИ ПРЫЖОК АКТИВЕН — ТОЛЬКО ВОЗДУШНОЕ УПРАВЛЕНИЕ ★★★★
	if (bIsJumping)
	{
		// Воздушное управление
		if (PlayerWantsToMove())
		{
			Vector moveDir = m_vecPlayerWishDir;
			Assassin_AirControl(moveDir);
		}

		// Продвигаем анимацию
		StudioFrameAdvance();
		DispatchAnimEvents(0.1);
		return;
	}

	// ★★★★ ЕСЛИ ХОТИМ АТАКОВАТЬ ИЛИ УЖЕ АТАКУЕМ — НЕ ДВИГАЕМСЯ ★★★★
	if (bWantMelee || bWantShoot || bIsAttacking)
	{
		pev->velocity.x = 0;
		pev->velocity.y = 0;
		RouteClear();
		m_bAIMoving = FALSE;
		m_bIsSliding = FALSE;

		if (bIsAttacking && !m_fSequenceFinished)
		{
			StudioFrameAdvance();
			DispatchAnimEvents(0.1);
			return;
		}
	}
	else
	{
		// ★★★★ НЕТ АТАКИ И НЕТ ПРЫЖКА — ОБЫЧНОЕ ДВИЖЕНИЕ ★★★★
		UpdateAIMovement();

		if (PlayerWantsToMove())
		{
			Activity correctActivity = m_bPlayerWalking ? ACT_WALK : ACT_RUN;
			if (m_Activity != correctActivity &&
				m_Activity != ACT_IDLE &&
				m_Activity != ACT_RANGE_ATTACK1 &&
				m_Activity != ACT_MELEE_ATTACK1 &&
				m_Activity != ACT_MELEE_ATTACK2)
			{
				SetActivity(correctActivity);
			}
		}
		else
		{
			if (m_Activity == ACT_RUN || m_Activity == ACT_WALK)
			{
				SetActivity(ACT_IDLE);
			}
		}
	}

	// ★★★★ СБРОС АТАКИ ПО ЗАВЕРШЕНИИ ★★★★
	if (bIsAttacking && m_fSequenceFinished)
	{
		SetActivity(ACT_IDLE);
		m_IdealActivity = ACT_IDLE;
	}

	// ★★★★ ЗАПУСК MELEE АТАКИ ★★★★
	if (bWantMelee)
	{
		if (m_fSequenceFinished ||
			(m_Activity != ACT_MELEE_ATTACK1 && m_Activity != ACT_MELEE_ATTACK2))
		{
			PerformMeleeAttack();
		}
	}
	else
	{
		if ((m_Activity == ACT_MELEE_ATTACK1 || m_Activity == ACT_MELEE_ATTACK2) &&
			m_fSequenceFinished)
		{
			SetActivity(ACT_IDLE);
		}
	}

	// ★★★★ ЗАПУСК СТРЕЛЬБЫ ★★★★
	if (bWantShoot)
	{
		if (m_fSequenceFinished || m_Activity != ACT_RANGE_ATTACK1)
		{
			Shoot();
		}
	}
	else
	{
		if (m_Activity == ACT_RANGE_ATTACK1 && m_fSequenceFinished)
		{
			SetActivity(ACT_IDLE);
		}
	}
}











void CHAssassin::UpdateAIMovement(void)
{
	if (!PlayerWantsToMove()) {
		// Остановка движения
		if (m_bAIMoving) {
			m_movementGoal = MOVEGOAL_NONE;
			RouteClear();
			m_bAIMoving = FALSE;
			m_bIsSliding = FALSE;
			if (m_Activity != ACT_IDLE)
				SetActivity(ACT_IDLE);
		}
		return;
	}

	// ★★★ ИСПОЛЬЗУЕМ УЖЕ ГОТОВЫЙ МИРОВОЙ ВЕКТОР ★★★
	Vector worldMoveDir = m_vecPlayerWishDir;

	// ★★★ РАСЧЁТ ДИСТАНЦИИ В ЗАВИСИМОСТИ ОТ РЕЖИМА ★★★
	float checkDistance = m_bPlayerWalking ? 32.0f : 64.0f;

	// ★★★ УЧИТЫВАЕМ ВЕРТИКАЛЬНУЮ СОСТАВЛЯЮЩУЮ ★★★
	// Если есть вертикальная компонента, добавляем её к цели
	Vector targetPos = pev->origin + worldMoveDir * checkDistance;

	// Если смотрим вверх или вниз, корректируем цель по вертикали
	if (fabs(m_flPlayerWishPitch) > 5.0f) {
		// Проецируем направление на горизонтальную плоскость для движения
		Vector horizontalDir = worldMoveDir;
		horizontalDir.z = 0;
		if (horizontalDir.Length() > 0) {
			horizontalDir = horizontalDir.Normalize();
			// Сохраняем горизонтальное движение, но добавляем вертикальную компоненту
			targetPos = pev->origin + horizontalDir * checkDistance;
			// Добавляем вертикальное смещение только если смотрим вверх/вниз
			targetPos.z += worldMoveDir.z * checkDistance * 0.5f;
		}
	}

	float currentSpeed = GetCurrentSpeed();
	Activity wantedActivity = m_bPlayerWalking ? ACT_WALK : ACT_RUN;

	// ★★★ ПЫТАЕМСЯ ИСПОЛЬЗОВАТЬ AI-ДВИЖЕНИЕ ★★★
	m_movementActivity = wantedActivity;

	if (MoveToLocation(wantedActivity, 0.1f, targetPos)) {
		m_vecLastAIGoal = targetPos;
		m_bAIMoving = TRUE;
		m_movementActivity = wantedActivity;
		m_bIsSliding = FALSE;

		if (m_Activity != wantedActivity) {
			SetActivity(wantedActivity);
		}

		// Взаимодействие с объектами
		Assassin_CheckObjectInteraction();
		Assassin_PhysicsInteraction();
	}
	else {
		// ★★★ СКОЛЬЖЕНИЕ С УЧЁТОМ ВЕРТИКАЛИ ★★★
		// Если AI не может построить путь, используем прямое движение
		Vector moveVelocity = worldMoveDir * currentSpeed;

		// Проверяем, не пытаемся ли мы идти в стену
		TraceResult tr;
		Vector checkPos = pev->origin + worldMoveDir * 16.0f;
		UTIL_TraceLine(pev->origin + Vector(0, 0, 16), checkPos + Vector(0, 0, 16),
			dont_ignore_monsters, ENT(pev), &tr);

		if (tr.flFraction < 1.0f) {
			// Блокировано - пробуем двигаться вдоль стены
			Vector slideDir = worldMoveDir - tr.vecPlaneNormal *
				DotProduct(worldMoveDir, tr.vecPlaneNormal);
			if (slideDir.Length() > 0.1f) {
				slideDir = slideDir.Normalize();
				moveVelocity = slideDir * currentSpeed;
			}
			else {
				moveVelocity = Vector(0, 0, 0);
			}
		}

		pev->velocity.x = moveVelocity.x;
		pev->velocity.y = moveVelocity.y;

		// ★★★ СОХРАНЯЕМ ВЕРТИКАЛЬНУЮ СКОРОСТЬ ДЛЯ ПРЫЖКОВ ★★★
		if (fabs(worldMoveDir.z) > 0.1f && (pev->flags & FL_ONGROUND)) {
			// Если стоим на земле и хотим двигаться вертикально - прыгаем
			if (worldMoveDir.z > 0) {
				// Прыжок вверх
				g_bJumpCharging = true;
				g_flJumpChargeAmount = 0.5f; // Средний прыжок
			}
		}

		m_bAIMoving = TRUE;
		m_bIsSliding = TRUE;

		Assassin_CheckObjectInteraction();
		Assassin_PhysicsInteraction();

		if (pev->flags & FL_ONGROUND) {
			if (m_Activity != wantedActivity) {
				SetActivity(wantedActivity);
			}
		}
	}
}












	
// ★★★★ ИНИЦИАЛИЗАЦИЯ СВОБОДНОЙ КАМЕРЫ СМЕРТИ ★★★★
void InitializeDeathCamera(CBasePlayer* pPlayer) {
	if (!g_pAssassin || !pPlayer) return;

	// ★★★★ ФИКСИРОВАННЫЕ ПАРАМЕТРЫ ДЛЯ ТЕСТА ★★★★
	g_flDeathCameraDistance = 120.0f;
	g_flDeathCameraHeight = 60.0f;

	// ★★★★ НАЧАЛЬНАЯ ПОЗИЦИЯ - СЗАДИ АССАСИНА ★★★★
	Vector assassinPos = g_pAssassin->pev->origin;
	Vector startCameraPos = assassinPos + Vector(-120.0f, 0, 60.0f);

	pPlayer->pev->origin = startCameraPos;

	// Взгляд на ассасина
	Vector lookDirection = assassinPos - startCameraPos;
	Vector lookAngles = UTIL_VecToAngles(lookDirection);

	pPlayer->pev->v_angle = lookAngles;
	pPlayer->pev->angles = Vector(0, lookAngles.y, 0);
	pPlayer->pev->fixangle = TRUE;

	g_bDeathCameraInitialized = true;
	ALERT(at_console, "Death camera started at fixed position behind assassin\n");
}


// ★★★★ ПРОСТОЙ ВАРИАНТ - РУЧНОЕ ВРАЩЕНИЕ ★★★★
void UpdateDeathCamera(CBasePlayer* pPlayer) {
	if (!g_pAssassin || !g_pAssassin->pev || !pPlayer) return;

	static bool firstFrame = true;
	static Vector initialCameraOffset;

	if (!g_bDeathCameraInitialized) {
		// ★★★★ ИНИЦИАЛИЗАЦИЯ НА ОСНОВЕ ТЕКУЩЕЙ КАМЕРЫ ★★★★
		Vector assassinPos = g_pAssassin->pev->origin;
		Vector cameraPos = pPlayer->pev->origin;

		// Сохраняем начальное смещение камеры относительно ассасина
		initialCameraOffset = cameraPos - assassinPos;

		// ★★★★ ПОЛНОСТЬЮ ФИКСИРУЕМ АССАСИНА ★★★★
		g_pAssassin->pev->movetype = MOVETYPE_NONE;
		g_pAssassin->pev->solid = SOLID_NOT;
		g_pAssassin->pev->velocity = Vector(0, 0, 0);
		g_pAssassin->pev->avelocity = Vector(0, 0, 0);
		g_pAssassin->pev->fixangle = TRUE;

		g_bDeathCameraInitialized = true;
		firstFrame = true;
		ALERT(at_console, "Death camera based on existing system initialized\n");
	}

	Vector assassinPos = g_pAssassin->pev->origin;
	Vector mouseAngles = pPlayer->pev->v_angle;

	// ★★★★ ИСПОЛЬЗУЕМ СУЩЕСТВУЮЩУЮ ЛОГИКУ КАМЕРЫ ★★★★
	// Параметры камеры (как в обычном режиме)
	float maxDistance = 120.0f;
	float minDistance = 30.0f;
	float maxHeight = 40.0f;
	float minHeight = 10.0f;
	float baseHeight = 25.0f;

	Vector forward, right, up;
	g_engfuncs.pfnAngleVectors(mouseAngles, forward, right, up);

	// ★★★★ ВЫЧИСЛЯЕМ ЖЕЛАЕМУЮ ПОЗИЦИЮ КАМЕРЫ ★★★★
	float verticalAngle = mouseAngles.x;

	// Линейная зависимость расстояния от угла (как в обычной камере)
	float desiredDistance = maxDistance;
	if (verticalAngle < 0.0f) {
		float angleRatio = fabs(verticalAngle) / 89.0f;
		desiredDistance = maxDistance - (maxDistance - minDistance) * angleRatio;
	}

	// Линейная зависимость высоты от угла
	float desiredHeight = baseHeight;
	if (verticalAngle > 0.0f) {
		float angleRatio = verticalAngle / 89.0f;
		desiredHeight = baseHeight + (maxHeight - baseHeight) * angleRatio;
	}
	else if (verticalAngle < 0.0f) {
		float angleRatio = fabs(verticalAngle) / 89.0f;
		desiredHeight = baseHeight - (baseHeight - minHeight) * angleRatio;
	}

	Vector desiredCameraPos = assassinPos + (-forward * desiredDistance) + (up * desiredHeight);

	// ★★★★ ТРАССИРОВКА СТОЛКНОВЕНИЙ (как в обычной камере) ★★★★
	float assassinHeadHeight = 64.0f;
	float cameraHeadHeight = 28.0f;

	Vector traceStart = assassinPos + Vector(0, 0, assassinHeadHeight);
	Vector desiredHeadPos = desiredCameraPos + Vector(0, 0, cameraHeadHeight);

	TraceResult tr;
	UTIL_TraceHull(traceStart, desiredHeadPos, ignore_monsters, point_hull, pPlayer->edict(), &tr);

	Vector targetCameraPos = desiredCameraPos;

	if (tr.flFraction < 1.0f) {
		// Корректировка при столкновении (как в обычной камере)
		Vector collisionPoint = tr.vecEndPos;
		Vector collisionDirection = (collisionPoint - traceStart).Normalize();
		Vector targetHeadPos = collisionPoint - collisionDirection * 1.0f;
		targetCameraPos = targetHeadPos - Vector(0, 0, cameraHeadHeight);
	}

	// ★★★★ ПЛАВНОЕ ПЕРЕМЕЩЕНИЕ КАМЕРЫ ★★★★
	Vector currentCameraPos = pPlayer->pev->origin;
	float distanceToTarget = (targetCameraPos - currentCameraPos).Length();
	float interpolationFactor = 0.3f;
	if (distanceToTarget < 10.0f) {
		interpolationFactor = 0.8f;
	}

	Vector newCameraPos = currentCameraPos + (targetCameraPos - currentCameraPos) * interpolationFactor;

	// ★★★★ ОБНОВЛЯЕМ КАМЕРУ ★★★★
	pPlayer->pev->origin = newCameraPos;

	// ★★★★ УГЛЫ АССАСИНА НЕ МЕНЯЕМ - ОН ФИКСИРОВАН ★★★★
	// Но камера может свободно вращаться

	ALERT(at_console, "Death camera - Mouse: (%.1f, %.1f), Pos: (%.1f, %.1f, %.1f)\n",
		mouseAngles.x, mouseAngles.y, newCameraPos.x, newCameraPos.y, newCameraPos.z);
}


// ★★★★ ФУНКЦИЯ СМЕРТИ АССАСИНА В РЕЖИМЕ УПРАВЛЕНИЯ ★★★★
void Assassin_HandleDeath(void) {
	if (!g_pAssassin || !g_bIsActive) return;

	if (g_bAssassinDeathSequence) {
		ALERT(at_console, "Death sequence already in progress\n");
		return;
	}

	if (!FStrEq(STRING(g_pAssassin->pev->classname), "monster_human_assassin"))
		return;

	CHAssassin* pAssassin = (CHAssassin*)g_pAssassin;

	ALERT(at_console, "Assassin death sequence started - disabling controls\n");

	// ★★★★ ОТКЛЮЧАЕМ УПРАВЛЕНИЕ ДВИЖЕНИЕМ ★★★★
	g_bMoveForward = false;
	g_bMoveBackward = false;
	g_bMoveLeft = false;
	g_bMoveRight = false;
	g_bJumpCharging = false;

	// ★★★★ ФИКСИРУЕМ АССАСИНА ★★★★
	pAssassin->pev->movetype = MOVETYPE_NONE;
	pAssassin->pev->solid = SOLID_NOT;
	pAssassin->pev->velocity = Vector(0, 0, 0);
	pAssassin->pev->avelocity = Vector(0, 0, 0);
	pAssassin->pev->deadflag = DEAD_DYING;

	// ★★★★ АНИМАЦИЯ СМЕРТИ ★★★★
	int deathSequence = pAssassin->LookupSequence("die");
	if (deathSequence == -1) {
		deathSequence = pAssassin->LookupSequence("crouch");
	}

	if (deathSequence != -1) {
		pAssassin->pev->sequence = deathSequence;
		pAssassin->pev->frame = 0;
		pAssassin->ResetSequenceInfo();
	}

	// ★★★★ ЗВУК СМЕРТИ ★★★★
	EMIT_SOUND(pAssassin->edict(), CHAN_VOICE, "player/pl_die1.wav", 1.0, ATTN_NORM);

	ALERT(at_console, "Assassin immobilized and controls disabled\n");
}


void UpdateAssassinDeathSequence(CBasePlayer* pPlayer) {
	if (!g_bAssassinDeathSequence || !g_bIsActive) return;

	// ★★★★ ИСПОЛЬЗУЕМ СФЕРИЧЕСКУЮ КАМЕРУ ★★★★
	UpdateDeathCamera(pPlayer);

	// Проверяем прошло ли 5 секунд
	if (gpGlobals->time - g_flAssassinDeathTime >= 5.0f) {
		ALERT(at_console, "Death sequence completed after %.1f seconds, stopping camera\n",
			gpGlobals->time - g_flAssassinDeathTime);
		StopHassassinCamera(pPlayer);
	}
}


// Функция обновления позиции камеры
void UpdateCameraPosition(CBasePlayer* pPlayer) {

	if (!g_pAssassin || !g_bIsActive) return;



	// ★★★★ ПРИНУДИТЕЛЬНАЯ ОТРИСОВКА BBOX ИЗ КАМЕРЫ ★★★★
	static float lastDrawTime = 0;
	if (g_bDrawAssassinBBox && (gpGlobals->time - lastDrawTime > 0.1f))
	{
		extern void DebugDrawAssassinFullBBox(void);
		DebugDrawAssassinFullBBox();
		lastDrawTime = gpGlobals->time;
	}


	// ★★★★ ЕСЛИ ИДЕТ СМЕРТЬ - ИСПОЛЬЗУЕМ КАМЕРУ СМЕРТИ ★★★★
	if (g_bAssassinDeathSequence) {
		UpdateAssassinDeathSequence(pPlayer);
		return;
	}


	// ★★★★ ПРОВЕРКА АССАСИНА ПЕРЕД ОБНОВЛЕНИЕМ ★★★★
	if (!g_pAssassin || !g_pAssassin->pev || (g_pAssassin->pev->flags & FL_KILLME)) {
		// Если ассасин умер, но еще не начали последовательность смерти
		if (!g_bAssassinDeathHandled && g_pAssassin && g_pAssassin->pev) {
			ALERT(at_console, "Assassin died outside of health sync - starting death sequence\n");
			Assassin_HandleDeath();
			g_bAssassinDeathSequence = true;
			g_bAssassinDeathHandled = true;
			g_flAssassinDeathTime = gpGlobals->time;
		}
		else {
			ALERT(at_console, "Camera update skipped - assassin invalid\n");
			StopHassassinCamera(pPlayer);
			return;
		}
	}

	// ★★★★ СИНХРОНИЗАЦИЯ ЗДОРОВЬЯ ДЛЯ HUD ★★★★
	UpdateAssassinHealthSync(pPlayer);

	// Если после синхронизации режим отключился - выходим
	if (!g_bIsActive) return;


	Vector mouseAngles = pPlayer->pev->v_angle;
	Vector assassinPos = g_pAssassin->pev->origin;
	Vector currentCameraPos = pPlayer->pev->origin;

	// Параметры камеры
	float maxDistance = 120.0f;
	float minDistance = 30.0f;
	float maxHeight = 40.0f;
	float minHeight = 10.0f;
	float baseHeight = 25.0f;
	float collisionOffset = 1.0f;

	Vector forward, right, up;
	g_engfuncs.pfnAngleVectors(mouseAngles, forward, right, up);



	// 1. ОБНОВЛЯЕМ УГЛЫ ПЕРСОНАЖА
	g_pAssassin->pev->angles = Vector(0, mouseAngles.y, 0);
	g_pAssassin->pev->v_angle = mouseAngles;
	g_pAssassin->pev->fixangle = TRUE;

	float verticalAngle = mouseAngles.x;

	// 2. ЛИНЕЙНАЯ ЗАВИСИМОСТЬ РАССТОЯНИЯ ОТ УГЛА
	float desiredDistance = maxDistance;
	if (verticalAngle < 0.0f) {
		float angleRatio = fabs(verticalAngle) / 89.0f;
		desiredDistance = maxDistance - (maxDistance - minDistance) * angleRatio;
	}

	// 3. ЛИНЕЙНАЯ ЗАВИСИМОСТЬ ВЫСОТЫ ОТ УГЛА
	float desiredHeight = baseHeight;
	if (verticalAngle > 0.0f) {
		float angleRatio = verticalAngle / 89.0f;
		desiredHeight = baseHeight + (maxHeight - baseHeight) * angleRatio;
	}
	else if (verticalAngle < 0.0f) {
		float angleRatio = fabs(verticalAngle) / 89.0f;
		desiredHeight = baseHeight - (baseHeight - minHeight) * angleRatio;
	}

	// 4. ВЫЧИСЛЯЕМ ЖЕЛАЕМУЮ ПОЗИЦИЮ КАМЕРЫ
	Vector desiredCameraPos = assassinPos + (-forward * desiredDistance) + (up * desiredHeight);

	// 5. ВЫЧИСЛЯЕМ ПОЗИЦИЮ ГОЛОВЫ КАМЕРЫ
	float cameraHeadHeight = 28.0f;
	Vector desiredHeadPos = desiredCameraPos + Vector(0, 0, cameraHeadHeight);

	// 6. ТРАССИРОВКА ОТ ГОЛОВЫ АССАСИНА К ГОЛОВЕ КАМЕРЫ
	float assassinHeadHeight = 64.0f;
	Vector traceStart = assassinPos + Vector(0, 0, assassinHeadHeight);

	TraceResult tr;
	UTIL_TraceHull(
		traceStart,
		desiredHeadPos,
		ignore_monsters,
		point_hull,
		pPlayer->edict(),
		&tr
	);

	Vector targetCameraPos = desiredCameraPos;

	// 7. ПРИЛИПАНИЕ К ТОЧКЕ СТОЛКНОВЕНИЯ
	if (tr.flFraction < 1.0f) {
		Vector collisionPoint = tr.vecEndPos;
		Vector collisionDirection = (collisionPoint - traceStart).Normalize();
		Vector targetHeadPos = collisionPoint - collisionDirection * collisionOffset;
		targetCameraPos = targetHeadPos - Vector(0, 0, cameraHeadHeight);
	}

	// 8. ПРОВЕРКА ЧТО КАМЕРА НЕ ВНУТРИ ГЕОМЕТРИИ
	TraceResult trCameraCheck;
	UTIL_TraceHull(
		targetCameraPos - Vector(1, 1, 1),
		targetCameraPos + Vector(1, 1, 1),
		dont_ignore_monsters,
		point_hull,
		pPlayer->edict(),
		&trCameraCheck
	);

	if (trCameraCheck.fStartSolid) {
		if (tr.flFraction < 1.0f) {
			Vector correctedHeadPos = tr.vecEndPos + tr.vecPlaneNormal * collisionOffset;
			targetCameraPos = correctedHeadPos - Vector(0, 0, cameraHeadHeight);
		}
	}

	// 9. ПЛАВНОЕ ПЕРЕМЕЩЕНИЕ
	float distanceToTarget = (targetCameraPos - currentCameraPos).Length();
	float interpolationFactor = 0.3f;
	if (distanceToTarget < 10.0f) {
		interpolationFactor = 0.8f;
	}

	Vector newCameraPos = currentCameraPos + (targetCameraPos - currentCameraPos) * interpolationFactor;
	pPlayer->pev->origin = newCameraPos;

	// 10. ОТЛАДОЧНАЯ ОТРИСОВКА (если включена)
	if (debug_traceline.value != 0.0f) {
		DebugDrawTraceLines(pPlayer);
	}

}


CBaseEntity* Assassin_FindUseEntity(void)
{
	if (!g_bIsActive || !g_pAssassin)
		return NULL;

	ALERT(at_console, "!!! ASSASSIN_FINDUSEENTITY - TRACING FROM ASSASSIN HEAD (BUTTONS ONLY) !!!\n");

	// Используем глобальный g_pAssassin как CBaseEntity
	CBaseEntity* pAssassin = g_pAssassin;

	// ★★★★ ТРАССИРОВКА ОТ ГОЛОВЫ АССАСИНА ★★★★
	TraceResult tr;
	Vector forward;

	// Используем углы ассасина
	UTIL_MakeVectors(pAssassin->pev->v_angle);
	forward = gpGlobals->v_forward;

	// ★★★★ ПОЗИЦИЯ ИЗ ГОЛОВЫ АССАСИНА ★★★★
	// Получаем высоту модели ассасина и берем 80% от нее для головы
	Vector headPosition = pAssassin->pev->origin;
	headPosition.z += pAssassin->pev->maxs.z * 0.8f; // Голова (80% высоты модели)

	Vector traceStart = headPosition;
	Vector traceEnd = traceStart + forward * 96.0f; // Дистанция использования

	ALERT(at_console, "Assassin head pos: (%.1f, %.1f, %.1f)\n",
		traceStart.x, traceStart.y, traceStart.z);
	ALERT(at_console, "Assassin height: %.1f units\n", pAssassin->pev->maxs.z);
	ALERT(at_console, "Trace direction: (%.1f, %.1f, %.1f) -> (%.1f, %.1f, %.1f)\n",
		traceStart.x, traceStart.y, traceStart.z,
		traceEnd.x, traceEnd.y, traceEnd.z);

	// Визуализация луча от головы ассасина (СИНИЙ - чтобы отличать)
	MESSAGE_BEGIN(MSG_ALL, SVC_TEMPENTITY);
	WRITE_BYTE(TE_BEAMPOINTS);
	WRITE_COORD(traceStart.x);
	WRITE_COORD(traceStart.y);
	WRITE_COORD(traceStart.z);
	WRITE_COORD(traceEnd.x);
	WRITE_COORD(traceEnd.y);
	WRITE_COORD(traceEnd.z);
	WRITE_SHORT(g_sModelIndexLaser);
	WRITE_BYTE(0); WRITE_BYTE(0); WRITE_BYTE(10); WRITE_BYTE(15); WRITE_BYTE(0); // Более тонкий луч
	WRITE_BYTE(0); WRITE_BYTE(100); WRITE_BYTE(255); // СИНИЙ цвет - из головы
	WRITE_BYTE(255); WRITE_BYTE(0);
	MESSAGE_END();

	UTIL_TraceLine(traceStart, traceEnd, dont_ignore_monsters, pAssassin->edict(), &tr);

	ALERT(at_console, "Trace fraction: %.3f\n", tr.flFraction);

	if (tr.flFraction < 1.0f && tr.pHit)
	{
		CBaseEntity* pHitEntity = CBaseEntity::Instance(tr.pHit);
		if (pHitEntity && pHitEntity != pAssassin)
		{
			const char* className = STRING(pHitEntity->pev->classname);
			ALERT(at_console, "Assassin found entity: %s\n", className);

			// ★★★★ ФИЛЬТРАЦИЯ: ТОЛЬКО КНОПКИ ★★★★
			BOOL bIsButton = FALSE;

			// Разрешенные типы кнопок:
			if (strcmp(className, "func_button") == 0 ||
				strcmp(className, "func_rot_button") == 0 ||
				strcmp(className, "momentary_rot_button") == 0)
			{
				bIsButton = TRUE;
				ALERT(at_console, ">>> BUTTON DETECTED - APPROVED\n");
			}
			else
			{
				ALERT(at_console, ">>> NOT A BUTTON - REJECTED: %s\n", className);
			}

			if (bIsButton)
			{
				float distance = (tr.vecEndPos - traceStart).Length();
				if (distance < 120.0f) // ★★★★ УВЕЛИЧЕННАЯ ДИСТАНЦИЯ ДЛЯ ГОЛОВЫ ★★★★
				{
					ALERT(at_console, "Button within range: %.1f units\n", distance);
					return pHitEntity;
				}
				else
				{
					ALERT(at_console, "Button too far: %.1f > 120.0\n", distance);
				}
			}
		}
	}

	ALERT(at_console, "Assassin found no usable buttons from head position\n");
	return NULL;
}


// Функция для временного превращения ассасина в игрока
void Assassin_MakePlayerLike(void)
{
	if (!g_pAssassin) return;

	CHAssassin* pAssassin = (CHAssassin*)g_pAssassin;

	// Сохраняем оригинальное состояние

	ALERT(at_console, "Making assassin player-like for interaction\n");

	// ★★★★ ДЕЛАЕМ АССАСИНА ПОХОЖИМ НА ИГРОКА ★★★★
	pAssassin->pev->classname = MAKE_STRING("player");
	pAssassin->pev->flags |= FL_CLIENT;        // Флаг игрока
	pAssassin->pev->flags &= ~FL_MONSTER;      // Убираем флаг монстра

	// Устанавливаем правильную классификацию
	pAssassin->pev->playerclass = 0;

	// Для триггеров и дверей
	pAssassin->pev->flags |= FL_ONGROUND;

	// Устанавливаем правильный solid для взаимодействия
	pAssassin->pev->solid = SOLID_SLIDEBOX;
}


// Функция восстановления оригинального состояния
void Assassin_RestoreOriginal(void)
{
	if (!g_pAssassin) return;

	CHAssassin* pAssassin = (CHAssassin*)g_pAssassin;

	// Восстанавливаем solid
	pAssassin->pev->solid = SOLID_SLIDEBOX;
}


bool Assassin_CanUse(void)
{
	static float s_flLastUse = 0;

	if (gpGlobals->time - s_flLastUse < 1.0f)
		return false;

	s_flLastUse = gpGlobals->time;
	return true;
}


void Assassin_ActivateButtonDirectly(CBaseEntity* pButton)
{
	if (!pButton || !g_pAssassin) return;

	ALERT(at_console, "DIRECT BUTTON ACTIVATION\n");

	// ★★★★ ПРЯМОЕ УПРАВЛЕНИЕ КНОПКОЙ ★★★★

	// 1. Меняем состояние кнопки
	pButton->pev->frame = 1; // Активировано

	// 2. Запускаем звук кнопки
	EMIT_SOUND(pButton->edict(), CHAN_VOICE, "buttons/button1.wav", 1.0, ATTN_NORM);

	// 3. Запускаем все targets кнопки
	const char* target = STRING(pButton->pev->target);
	if (target && strlen(target) > 0)
	{
		ALERT(at_console, "Firing targets: %s\n", target);

		// Ищем все entity с указанным targetname и активируем их
		CBaseEntity* pTarget = NULL;
		while ((pTarget = UTIL_FindEntityByTargetname(pTarget, target)) != NULL)
		{
			ALERT(at_console, "Activating target: %s\n", STRING(pTarget->pev->classname));
			pTarget->Use(g_pAssassin, g_pAssassin, USE_TOGGLE, 0);
		}
	}

	// 4. Звук подтверждения для ассасина
	EMIT_SOUND(g_pAssassin->edict(), CHAN_ITEM, "buttons/button1.wav", 0.7, ATTN_NORM);

	ALERT(at_console, "Button activated successfully\n");
}


// Основная функция использования от ассасина
void Assassin_CheckUse(void)
{
	// Защита от спама
	if (gpGlobals->time - g_flLastAssassinUse < g_flUseCooldown)
		return;

	if (!g_bIsActive || !g_pAssassin)
	{
		ALERT(at_console, "ERROR: Assassin system not active\n");
		return;
	}

	ALERT(at_console, "!!! ASSASSIN USE - TRACING FROM ASSASSIN POSITION !!!\n");

	CHAssassin* pAssassin = (CHAssassin*)g_pAssassin;

	// ★★★★ ТРАССИРОВКА ОТ АССАСИНА - УЛУЧШЕННАЯ ★★★★
	TraceResult tr;

	// Используем текущие углы ассасина
	Vector assassinAngles = pAssassin->pev->angles;
	assassinAngles.x = 0; // Обнуляем pitch для горизонтального луча

	UTIL_MakeVectors(assassinAngles);
	Vector forward = gpGlobals->v_forward;

	// Трассируем от центра ассасина
	Vector traceStart = pAssassin->pev->origin + Vector(0, 0, 36); // Уровень груди
	Vector traceEnd = traceStart + forward * 128.0f; // Увеличенная дистанция

	ALERT(at_console, "Assassin pos: (%.1f, %.1f, %.1f)\n",
		traceStart.x, traceStart.y, traceStart.z);
	ALERT(at_console, "Trace direction: (%.1f, %.1f, %.1f)\n",
		forward.x, forward.y, forward.z);

	// ★★★★ ВИЗУАЛИЗАЦИЯ - КРАСНЫЙ ЛУЧ ОТ АССАСИНА ★★★★
	MESSAGE_BEGIN(MSG_ALL, SVC_TEMPENTITY);
	WRITE_BYTE(TE_BEAMPOINTS);
	WRITE_COORD(traceStart.x);
	WRITE_COORD(traceStart.y);
	WRITE_COORD(traceStart.z);
	WRITE_COORD(traceEnd.x);
	WRITE_COORD(traceEnd.y);
	WRITE_COORD(traceEnd.z);
	WRITE_SHORT(g_sModelIndexLaser);
	WRITE_BYTE(0);        // start frame
	WRITE_BYTE(0);        // frame rate
	WRITE_BYTE(10);       // life in 0.1's
	WRITE_BYTE(10);       // line width in 0.1's
	WRITE_BYTE(0);        // noise amplitude in 0.01's
	WRITE_BYTE(255);      // RED
	WRITE_BYTE(0);        // GREEN
	WRITE_BYTE(0);        // BLUE
	WRITE_BYTE(255);      // brightness
	WRITE_BYTE(0);        // scroll speed in 0.1's
	MESSAGE_END();

	// ★★★★ ТРАССИРУЕМ ХАЛЛОМ ДЛЯ ЛУЧШЕГО ОБНАРУЖЕНИЯ ★★★★
	UTIL_TraceHull(traceStart, traceEnd, dont_ignore_monsters, head_hull, pAssassin->edict(), &tr);

	ALERT(at_console, "Trace fraction: %.3f, Hit: %s\n",
		tr.flFraction, tr.pHit ? "YES" : "NO");

	BOOL bObjectActivated = FALSE;

	if (tr.flFraction < 1.0f && tr.pHit)
	{
		CBaseEntity* pHitEntity = CBaseEntity::Instance(tr.pHit);
		if (pHitEntity && pHitEntity != pAssassin)
		{
			const char* className = STRING(pHitEntity->pev->classname);
			float distance = (tr.vecEndPos - traceStart).Length();

			ALERT(at_console, "Assassin found: %s at distance %.1f\n", className, distance);

			// ★★★★ ПРОВЕРКА ВСЕХ ВОЗМОЖНЫХ ТИПОВ ОБЪЕКТОВ ★★★★
			if (strstr(className, "button") != NULL ||
				strstr(className, "door") != NULL ||
				strstr(className, "train") != NULL ||
				strstr(className, "plat") != NULL ||
				strcmp(className, "func_rot_button") == 0 ||
				strcmp(className, "momentary_rot_button") == 0 ||
				strcmp(className, "func_tracktrain") == 0)
			{
				ALERT(at_console, ">>> ASSASSIN ACTIVATING: %s\n", className);

				// ★★★★ АКТИВАЦИЯ ОТ ИМЕНИ АССАСИНА ★★★★
				// Сохраняем оригинальные свойства
				int oldFlags = pAssassin->pev->flags;
				const char* oldClassname = STRING(pAssassin->pev->classname);

				// Временно делаем ассасина игроком
				pAssassin->pev->flags |= FL_CLIENT;
				pAssassin->pev->classname = MAKE_STRING("player");

				// Активируем объект
				pHitEntity->Use(pAssassin, pAssassin, USE_TOGGLE, 0);

				// Восстанавливаем свойства
				pAssassin->pev->flags = oldFlags;
				pAssassin->pev->classname = MAKE_STRING(oldClassname);

				// Звук успеха
				EMIT_SOUND(pAssassin->edict(), CHAN_ITEM, "buttons/button1.wav", 0.7, ATTN_NORM);
				ALERT(at_console, "*** ASSASSIN SUCCESS: Object activated ***\n");
				bObjectActivated = TRUE;
			}
			else
			{
				ALERT(at_console, "Object type not supported for activation: %s\n", className);
			}
		}
	}

	if (!bObjectActivated)
	{
		ALERT(at_console, "Assassin found no activatable object in range\n");
		EMIT_SOUND(pAssassin->edict(), CHAN_ITEM, "common/wpn_denyselect.wav", 0.8, ATTN_NORM);
	}

	// Обновляем время последнего использования
	g_flLastAssassinUse = gpGlobals->time;
}



























void StartHassassinCamera(CBasePlayer* pPlayer, CBaseEntity* pAssassin) {
	// ★★★★ ПРОВЕРКА, ЧТО АССАСИН УЖЕ НЕ УПРАВЛЯЕТСЯ ★★★★
	if (g_bAssassinControlled) {
		ALERT(at_console, "ERROR: Assassin is already being controlled!\n");
		return;
	}


	// ★★★★ ПРОВЕРКА, ЧТО ИГРОК УЖЕ НЕ В РЕЖИМЕ АССАСИНА ★★★★
	if (g_bIsActive) {
		ALERT(at_console, "ERROR: Player is already in assassin mode!\n");
		return;
	}


	// ★★★★ СБРАСЫВАЕМ ВСЕ ФЛАГИ СМЕРТИ ★★★★
	g_bAssassinDeathSequence = false;
	g_bAssassinDeathHandled = false;
	g_flAssassinDeathTime = 0.0f;



	ALERT(at_console, "StartHassassinCamera called\n");

	g_pAssassin = pAssassin;
	g_bIsActive = true;
	g_bAssassinControlled = true; // ★★★★ УСТАНАВЛИВАЕМ ФЛАГ ★★★★

	
	// ★★★ СБРАСЫВАЕМ СОСТОЯНИЕ НОВЫХ ФУНКЦИЙ ★★★
	g_bLastInvPressed = false;
	g_bReloadPressed = false;
	g_bInvisible = false;
	g_flLastGrenadeTime = 0;
	g_flLastInvisToggleTime = 0;
	g_flNextInvisToggleTime = 0;




	// Скрываем оружие игрока
	HidePlayerWeapons(pPlayer);



	// ★★★ СОХРАНЯЕМ СОСТОЯНИЕ ПРИСЕДАНИЯ ★★★
	SavePlayerDuckState(pPlayer);

	// ★★★ ОТКЛЮЧАЕМ ПРИСЕДАНИЕ ИГРОКА ★★★
	DisablePlayerDuck(pPlayer);





	// ★★★★ СОХРАНЯЕМ СОСТОЯНИЕ ИГРОКА ★★★★
	ALERT(at_console, "Player weapons bitfield BEFORE: %d\n", pPlayer->pev->weapons);
	ALERT(at_console, "Player health BEFORE: %.0f\n", pPlayer->pev->health);

	// Прячем игрока и уменьшаем его размеры
	pPlayer->pev->effects |= EF_NODRAW;
	pPlayer->pev->solid = SOLID_NOT;
	pPlayer->pev->movetype = MOVETYPE_NOCLIP;

	// Устанавливаем размеры как у гранаты (4x4x4)
	pPlayer->pev->mins = Vector(-2, -2, -2);
	pPlayer->pev->maxs = Vector(2, 2, 2);
	SET_SIZE(pPlayer->edict(), pPlayer->pev->mins, pPlayer->pev->maxs);

	// ★★★★ СИНХРОНИЗИРУЕМ ЗДОРОВЬЕ С АССАСИНОМ ★★★★
	if (g_pAssassin) {
		// Сохраняем оригинальное здоровье игрока в user данных
		pPlayer->pev->fuser1 = pPlayer->pev->health;
		pPlayer->pev->fuser2 = pPlayer->pev->max_health;

		// Копируем здоровье ассасина игроку (для HUD)
		pPlayer->pev->health = g_pAssassin->pev->health;
		pPlayer->pev->max_health = g_pAssassin->pev->max_health;

		// Телепортируем камеру к ассасину
		Vector cameraPos = g_pAssassin->pev->origin;
		cameraPos.z += 36.0f;
		pPlayer->pev->origin = cameraPos;

		pPlayer->pev->angles = g_pAssassin->pev->angles;
		pPlayer->pev->v_angle = g_pAssassin->pev->v_angle;
		pPlayer->pev->fixangle = TRUE;
	}

	ALERT(at_console, "Assassin system activated - player state preserved\n");
}


void StopHassassinCamera(CBasePlayer* pPlayer) {
	ALERT(at_console, "StopHassassinCamera called\n");

	// Выключаем отрисовку BBox
	if (g_bDrawAssassinBBox) {
		g_bDrawAssassinBBox = false;
	}

	if (!g_bIsActive) return;

	// Сбрасываем флаги камеры смерти
	g_bDeathCameraInitialized = false;
	g_bAssassinDeathSequence = false;
	g_bAssassinDeathHandled = false;
	g_flAssassinDeathTime = 0.0f;
	g_vecDeathCameraAngles = Vector(0, 0, 0);

	// Безопасное восстановление игрока
	if (g_pAssassin && g_pAssassin->pev) {
		// Проверяем, не помечен ли уже на удаление
		if (!(g_pAssassin->pev->flags & FL_KILLME)) {
			Vector spawnPos = g_pAssassin->pev->origin;
			spawnPos.z += 36.0f;

			pPlayer->pev->origin = spawnPos;
			pPlayer->pev->angles = g_pAssassin->pev->angles;
			pPlayer->pev->v_angle = g_pAssassin->pev->v_angle;
			pPlayer->pev->fixangle = TRUE;

			// Восстанавливаем оригинальное здоровье
			if (pPlayer->pev->fuser1 > 0) {
				pPlayer->pev->health = pPlayer->pev->fuser1;
				pPlayer->pev->max_health = pPlayer->pev->fuser2;
			}

			// ★★★ ВОТ ГЛАВНОЕ ИСПРАВЛЕНИЕ ★★★
			// Удаляем ассасина, если он жив
			if (g_pAssassin->pev->health > 0 && g_pAssassin->pev->deadflag == DEAD_NO) {
				// Запускаем анимацию исчезновения (опционально)
				g_pAssassin->pev->effects |= EF_NODRAW;  // Мгновенно скрываем

				// Планируем удаление
				g_pAssassin->pev->nextthink = gpGlobals->time + 0.1f;
				g_pAssassin->SetThink(&CBaseEntity::SUB_Remove);

				ALERT(at_console, "Live assassin scheduled for removal\n");
			}
		}

		g_pAssassin = NULL;  // Обнуляем указатель ПОСЛЕ обработки
	}

	// Восстанавливаем свойства игрока
	pPlayer->pev->effects &= ~EF_NODRAW;
	pPlayer->pev->solid = SOLID_SLIDEBOX;
	pPlayer->pev->movetype = MOVETYPE_WALK;
	pPlayer->pev->mins = VEC_HUMAN_HULL_MIN;
	pPlayer->pev->maxs = VEC_HUMAN_HULL_MAX;
	SET_SIZE(pPlayer->edict(), pPlayer->pev->mins, pPlayer->pev->maxs);



	// ★★★ СБРАСЫВАЕМ НЕВИДИМОСТЬ ПРИ ВЫХОДЕ ★★★
	if (g_bInvisible && g_pAssassin && g_pAssassin->pev)
	{
		g_pAssassin->pev->rendermode = kRenderTransTexture;
		g_pAssassin->pev->renderamt = 20;
		g_pAssassin->pev->renderfx = kRenderFxNone;
		g_bInvisible = false;
	}

	// Показываем оружие игрока
	ShowPlayerWeapons(pPlayer);


	// ★★★ ВОССТАНАВЛИВАЕМ СОСТОЯНИЕ ПРИСЕДАНИЯ ★★★
	RestorePlayerDuckState(pPlayer);

	// Сбрасываем флаги
	g_bIsActive = false;
	g_bAssassinControlled = false;
	g_pAssassin = NULL;

	ALERT(at_console, "Camera system stopped safely, flags reset\n");
}





void UpdateAssassinHealthSync(CBasePlayer* pPlayer) {
	if (!g_bIsActive || !pPlayer) return;

	// ★★★★ ЕСЛИ УЖЕ ОБРАБАТЫВАЕМ СМЕРТЬ - ВЫХОДИМ ★★★★
	if (g_bAssassinDeathSequence) {
		return;
	}

	if (!g_pAssassin || !g_pAssassin->pev) {
		ALERT(at_console, "Assassin no longer exists - stopping camera\n");
		StopHassassinCamera(pPlayer);
		return;
	}

	if (g_pAssassin->pev->flags & FL_KILLME) {
		ALERT(at_console, "Assassin is marked for removal - stopping camera\n");
		StopHassassinCamera(pPlayer);
		return;
	}

	// Синхронизируем здоровье
	if (pPlayer->pev->health != g_pAssassin->pev->health) {
		pPlayer->pev->health = g_pAssassin->pev->health;
		ALERT(at_console, "Health synced to HUD: %.0f\n", pPlayer->pev->health);
	}

	// ★★★★ ПРОВЕРКА НАЧАЛА СМЕРТИ (ТОЛЬКО ЕСЛИ ЕЩЕ НЕ ОБРАБАТЫВАЕМ) ★★★★
	if ((g_pAssassin->pev->health <= 0 || g_pAssassin->pev->deadflag != DEAD_NO) &&
		!g_bAssassinDeathHandled) {

		ALERT(at_console, "Assassin death detected - health: %.0f, deadflag: %d\n",
			g_pAssassin->pev->health, g_pAssassin->pev->deadflag);

		// ★★★★ ЗАПУСКАЕМ ПОСЛЕДОВАТЕЛЬНОСТЬ СМЕРТИ ★★★★
		Assassin_HandleDeath();

		// ★★★★ УСТАНАВЛИВАЕМ ФЛАГИ ЗАДЕРЖКИ ★★★★
		g_bAssassinDeathSequence = true;
		g_bAssassinDeathHandled = true;
		g_flAssassinDeathTime = gpGlobals->time;

		ALERT(at_console, "Death sequence started at: %.2f\n", g_flAssassinDeathTime);
		return;
	}
}


// ★★★★ ФУНКЦИЯ ПРОВЕРКИ, МОЖНО ЛИ АКТИВИРОВАТЬ АССАСИНА ★★★★
bool CanActivateAssassin(void) {
	if (g_bAssassinControlled) {
		ALERT(at_console, "Cannot activate: assassin already controlled\n");
		return false;
	}

	if (g_bIsActive) {
		ALERT(at_console, "Cannot activate: already in assassin mode\n");
		return false;
	}

	if (g_pAssassin != NULL) {
		ALERT(at_console, "Cannot activate: g_pAssassin is not NULL\n");
		return false;
	}

	return true;
}


// ★★★★ ФУНКЦИЯ ПРИМЕНЕНИЯ УРОНА К АССАСИНУ ★★★★
void Assassin_TakeDamage(float damage) {
	if (!g_pAssassin || !g_bIsActive) return;

	CHAssassin* pAssassin = (CHAssassin*)g_pAssassin;

	ALERT(at_console, "Assassin taking damage: %.0f\n", damage);

	// Применяем урон
	pAssassin->pev->health -= damage;

	// ★★★★ ЕСЛИ ЗДОРОВЬЕ УПАЛО НИЖЕ 0 - ЗАПУСКАЕМ СМЕРТЬ ★★★★
	if (pAssassin->pev->health <= 0 && pAssassin->pev->deadflag == DEAD_NO) {
		ALERT(at_console, "Assassin killed by damage: %.0f\n", damage);
		pAssassin->pev->health = 0; // Гарантируем, что здоровье = 0

		// ★★★★ ВИЗУАЛЬНЫЙ ЭФФЕКТ УРОНА ★★★★
		pAssassin->pev->rendermode = kRenderTransTexture;
		pAssassin->pev->renderamt = 100;

		// Звук получения урона
		EMIT_SOUND(pAssassin->edict(), CHAN_VOICE, "player/pl_pain5.wav", 1.0, ATTN_NORM);
	}
}


void CHAssassin::PlayerControlledDeath(void)
{
	ALERT(at_console, "CHAssassin::PlayerControlledDeath called\n");

	// Отключаем AI и физику
	m_MonsterState = MONSTERSTATE_DEAD;
	pev->deadflag = DEAD_DYING;
	pev->movetype = MOVETYPE_NONE;
	pev->solid = SOLID_NOT;

	// Анимация смерти
	int deathAnim = LookupActivity(ACT_DIESIMPLE);
	if (deathAnim == -1)
		deathAnim = LookupSequence("die");

	if (deathAnim != -1) {
		pev->sequence = deathAnim;
		pev->frame = 0;
		ResetSequenceInfo();
	}

	// Звук смерти
	EMIT_SOUND(ENT(pev), CHAN_VOICE, "player/pl_die1.wav", 1.0, ATTN_NORM);
}











void Assassin_MoveWithJerk(void)
{
	if (!g_pAssassin) return;
	CHAssassin* pAssassin = (CHAssassin*)g_pAssassin;

	// Восстановление физики
	if (pAssassin->pev->movetype == MOVETYPE_TOSS && (pAssassin->pev->flags & FL_ONGROUND)) {
		pAssassin->pev->movetype = MOVETYPE_STEP;
	}

	bool isOnGround = (pAssassin->pev->flags & FL_ONGROUND) != 0;

	// Воздушное управление
	if (!isOnGround) {
		Vector moveDirection = CalculateMoveDirection();
		if (moveDirection.Length() > 0) {
			Assassin_AirControl(moveDirection);
		}
		return;
	}

	// Наземное движение
	Vector currentMoveDirection = CalculateMoveDirection();

	if (currentMoveDirection.Length() > 0) {
		// Преобразуем локальное направление в мировые координаты
		UTIL_MakeVectors(pAssassin->pev->angles);
		Vector worldMoveDir = (gpGlobals->v_forward * currentMoveDirection.x) +
			(gpGlobals->v_right * currentMoveDirection.y);
		worldMoveDir = worldMoveDir.Normalize();

		// ★★★★ ВЫБИРАЕМ СКОРОСТЬ В ЗАВИСИМОСТИ ОТ РЕЖИМА ★★★★
		float moveSpeed = pAssassin->IsWalking() ? g_flWalkSpeed : g_flRunSpeed;

		// Рассчитываем точку назначения
		Vector dest = pAssassin->pev->origin + worldMoveDir * moveSpeed * gpGlobals->frametime;

		// Используем встроенную функцию движения
		UTIL_MoveToOrigin(ENT(pAssassin->pev), dest, moveSpeed * gpGlobals->frametime, MOVE_NORMAL);

		// ★★★★ АНИМАЦИЯ В ЗАВИСИМОСТИ ОТ РЕЖИМА ★★★★
		Activity currentActivity = pAssassin->IsWalking() ? ACT_WALK : ACT_RUN;
		pAssassin->SetActivity(currentActivity);
	}
	else
	{
		// Остановка
		pAssassin->SetActivity(ACT_IDLE);
	}
}










void Assassin_PhysicsInteraction(void)
{
	if (!g_pAssassin) return;

	static float lastPhysicsTime = 0;
	if (gpGlobals->time - lastPhysicsTime < 0.2f) return;  // Уменьшил задержку

	CHAssassin* pAssassin = (CHAssassin*)g_pAssassin;

	// ★★★★ ПРОВЕРЯЕМ СТОЛКНОВЕНИЯ СО ВСЕХ СТОРОН С ИСПОЛЬЗОВАНИЕМ BBOX ★★★★
	Vector center = pAssassin->pev->origin;
	Vector mins = pAssassin->pev->mins;
	Vector maxs = pAssassin->pev->maxs;

	// Проверяем 8 направлений вокруг BBox
	Vector directions[8] = {
		Vector(16, 0, 0),    // Вперед
		Vector(-16, 0, 0),   // Назад
		Vector(0, 16, 0),    // Вправо
		Vector(0, -16, 0),   // Влево
		Vector(16, 16, 0),   // Вперед-вправо
		Vector(16, -16, 0),  // Вперед-влево
		Vector(-16, 16, 0),  // Назад-вправо
		Vector(-16, -16, 0)  // Назад-влево
	};

	for (int i = 0; i < 8; i++)
	{
		Vector start = center;
		Vector end = center + directions[i];

		TraceResult tr;
		UTIL_TraceLine(start, end, dont_ignore_monsters, pAssassin->edict(), &tr);

		if (tr.flFraction < 1.0f && tr.pHit)
		{
			CBaseEntity* pHitEntity = CBaseEntity::Instance(tr.pHit);
			if (pHitEntity && pHitEntity != pAssassin)
			{
				const char* className = STRING(pHitEntity->pev->classname);

				// Проверяем двери, кнопки и триггеры
				if (strncmp(className, "func_door", 9) == 0 ||
					strncmp(className, "func_door_rotating", 18) == 0 ||
					strncmp(className, "func_button", 11) == 0 ||
					strncmp(className, "trigger_", 8) == 0)
				{
					ALERT(at_console, "Physics: Assassin touching %s\n", className);

					Assassin_MakePlayerLike();
					pHitEntity->Touch(pAssassin);
					Assassin_RestoreOriginal();

					lastPhysicsTime = gpGlobals->time;
					return;
				}
			}
		}
	}
}


// Обработка взаимодействия с разными типами объектов
void HandleEntityInteraction(CBaseEntity* pEntity, const char* direction)
{
	if (!pEntity || !g_pAssassin) return;

	CHAssassin* pAssassin = (CHAssassin*)g_pAssassin;
	const char* className = STRING(pEntity->pev->classname);

	//ALERT(at_console, "Assassin interaction %s with: %s\n", direction, className);

	// ★★★★ ПРОСТО ВЫЗЫВАЕМ TOUCH ДЛЯ ЛЮБОГО ОБЪЕКТА ★★★★
	// Движок Half-Life сам обработает все условия
	pEntity->Touch(pAssassin);
}









// Вспомогательная функция для проверки и активации entity
void CheckAndTouchEntity(edict_t* pHit, CHAssassin* pAssassin)
{
	if (!pHit || !pAssassin) return;

	CBaseEntity* pHitEntity = CBaseEntity::Instance(pHit);
	if (!pHitEntity || pHitEntity == pAssassin) return;

	const char* className = STRING(pHitEntity->pev->classname);

	// Проверяем все типы объектов, которые должны реагировать
	if (strncmp(className, "func_door", 9) == 0 ||
		strncmp(className, "func_door_rotating", 18) == 0 ||
		strncmp(className, "func_button", 11) == 0 ||
		strncmp(className, "func_rot_button", 15) == 0 ||
		strncmp(className, "momentary_rot_button", 20) == 0 ||
		strncmp(className, "trigger_", 8) == 0 ||
		strncmp(className, "func_plat", 9) == 0 ||
		strncmp(className, "func_train", 10) == 0 ||
		strncmp(className, "func_tracktrain", 15) == 0)
	{
		ALERT(at_console, "Assassin touching: %s\n", className);

		// Временно превращаем ассасина в игрока
		Assassin_MakePlayerLike();

		// Вызываем Touch
		pHitEntity->Touch(pAssassin);

		// Восстанавливаем
		Assassin_RestoreOriginal();
	}
}






// Вспомогательная функция для активации двери
void ActivateDoor(CBaseEntity* pDoor, CHAssassin* pAssassin)
{
	if (!pDoor || !pAssassin) return;

	const char* className = STRING(pDoor->pev->classname);
	ALERT(at_console, "Activating door: %s\n", className);

	// Временно превращаем ассасина в игрока
	int oldFlags = pAssassin->pev->flags;
	const char* oldClassname = STRING(pAssassin->pev->classname);

	pAssassin->pev->flags |= FL_CLIENT;
	pAssassin->pev->classname = MAKE_STRING("player");

	// Вызываем Touch для двери
	pDoor->Touch(pAssassin);

	// Восстанавливаем
	pAssassin->pev->flags = oldFlags;
	pAssassin->pev->classname = MAKE_STRING(oldClassname);
}










// Добавляем функцию для проверки и взаимодействия с объектами
void Assassin_CheckObjectInteraction(void)
{
	if (!g_pAssassin) return;

	static float lastTouch = 0;
	if (gpGlobals->time - lastTouch < 0.1f) return;  // Уменьшил до 0.1 сек

	CHAssassin* pAssassin = (CHAssassin*)g_pAssassin;

	Vector center = pAssassin->pev->origin;
	Vector velocity = pAssassin->pev->velocity;

	// ★★★★ ОПРЕДЕЛЯЕМ НАПРАВЛЕНИЕ ДВИЖЕНИЯ ★★★★
	Vector moveDir;
	BOOL hasMoveInput = (g_bMoveForward || g_bMoveBackward || g_bMoveLeft || g_bMoveRight);

	if (hasMoveInput)
	{
		// Используем направление от клавиш
		UTIL_MakeVectors(pAssassin->pev->angles);
		moveDir = Vector(0, 0, 0);
		if (g_bMoveForward) moveDir = moveDir + gpGlobals->v_forward;
		if (g_bMoveBackward) moveDir = moveDir - gpGlobals->v_forward;
		if (g_bMoveRight) moveDir = moveDir + gpGlobals->v_right;
		if (g_bMoveLeft) moveDir = moveDir - gpGlobals->v_right;
		moveDir = moveDir.Normalize();
	}
	else if (velocity.Length() > 10.0f)
	{
		// Используем направление скорости
		moveDir = velocity.Normalize();
	}
	else
	{
		// Нет движения - не проверяем
		return;
	}

	// ★★★★ ПРЕДВАРИТЕЛЬНАЯ ПРОВЕРКА: смотрим на 40 единиц вперед ★★★★
	float checkDistance = 40.0f;  // Увеличил дистанцию предварительной проверки
	Vector eyePos = center + Vector(0, 0, 36);
	Vector traceEnd = eyePos + moveDir * checkDistance;

	TraceResult tr;
	UTIL_TraceLine(eyePos, traceEnd, dont_ignore_monsters, pAssassin->edict(), &tr);

	if (tr.flFraction < 1.0f && tr.pHit)
	{
		CBaseEntity* pHitEntity = CBaseEntity::Instance(tr.pHit);
		if (pHitEntity && pHitEntity != pAssassin)
		{
			const char* className = STRING(pHitEntity->pev->classname);

			// Проверяем двери
			if (strncmp(className, "func_door", 9) == 0 ||
				strncmp(className, "func_door_rotating", 18) == 0)
			{
				float distance = (tr.vecEndPos - eyePos).Length();
				ALERT(at_console, "Door detected at %.1f units ahead\n", distance);

				// Если дверь близко (менее 50 единиц), активируем заранее
				if (distance < 50.0f)
				{
					ALERT(at_console, "Pre-activating door!\n");
					ActivateDoor(pHitEntity, pAssassin);
					lastTouch = gpGlobals->time;
					return;
				}
			}
		}
	}

	// ★★★★ ПРОВЕРКА ТЕКУЩЕГО СТОЛКНОВЕНИЯ (4 направления) ★★★★
	Vector directions[4] = {
		Vector(20, 0, 0),   // Вперед
		Vector(-20, 0, 0),  // Назад
		Vector(0, 20, 0),   // Вправо
		Vector(0, -20, 0)   // Влево
	};

	for (int i = 0; i < 4; i++)
	{
		Vector start = center;
		Vector end = center + directions[i];

		UTIL_TraceLine(start, end, dont_ignore_monsters, pAssassin->edict(), &tr);

		if (tr.flFraction < 1.0f && tr.pHit)
		{
			CBaseEntity* pHitEntity = CBaseEntity::Instance(tr.pHit);
			if (pHitEntity && pHitEntity != pAssassin)
			{
				const char* className = STRING(pHitEntity->pev->classname);

				if (strncmp(className, "func_door", 9) == 0 ||
					strncmp(className, "func_door_rotating", 18) == 0 ||
					strncmp(className, "func_button", 11) == 0)
				{
					ActivateDoor(pHitEntity, pAssassin);
					lastTouch = gpGlobals->time;
					return;
				}
			}
		}
	}
}






// Добавляем в hassassin.cpp
bool Assassin_IsOnGround(void)
{
	if (!g_pAssassin) return true;
	return (g_pAssassin->pev->flags & FL_ONGROUND) != 0;
}

float Assassin_GetVelocityZ(void)
{
	if (!g_pAssassin) return 0;
	return g_pAssassin->pev->velocity.z;
}


int Assassin_GetSequence(void)
{
	if (!g_pAssassin) return -1;
	return g_pAssassin->pev->sequence;
}

void Assassin_SetActivity(int activity)
{
	if (!g_pAssassin) return;

	if (!FStrEq(STRING(g_pAssassin->pev->classname), "monster_human_assassin"))
		return;

	CHAssassin* pAssassin = (CHAssassin*)g_pAssassin;
	pAssassin->SetActivity((Activity)activity); // ПРАВИЛЬНОЕ ПРИВЕДЕНИЕ ТИПА
}


void DebugDrawAssassinFullBBox(void)
{
	if (!g_pAssassin || !g_bIsActive) return;

	CHAssassin* pAssassin = (CHAssassin*)g_pAssassin;
	Vector origin = pAssassin->pev->origin;
	Vector mins = pAssassin->pev->mins;
	Vector maxs = pAssassin->pev->maxs;

	ALERT(at_console, "DRAWING FULL BBOX at (%.1f, %.1f, %.1f)\n", origin.x, origin.y, origin.z);

	// Вычисляем реальные углы BBOX
	Vector bbox_min = origin + mins;
	Vector bbox_max = origin + maxs;

	// 1. ВЕРТИКАЛЬНЫЕ РЕБРА (4 угла) - КРАСНЫЕ
	MESSAGE_BEGIN(MSG_ALL, SVC_TEMPENTITY);
	WRITE_BYTE(TE_BEAMPOINTS);
	WRITE_COORD(bbox_min.x); WRITE_COORD(bbox_min.y); WRITE_COORD(bbox_min.z);
	WRITE_COORD(bbox_min.x); WRITE_COORD(bbox_min.y); WRITE_COORD(bbox_max.z);
	WRITE_SHORT(g_sModelIndexLaser);
	WRITE_BYTE(0); WRITE_BYTE(0); WRITE_BYTE(10); WRITE_BYTE(5); WRITE_BYTE(0);
	WRITE_BYTE(255); WRITE_BYTE(0); WRITE_BYTE(0); WRITE_BYTE(255); WRITE_BYTE(0);
	MESSAGE_END();

	MESSAGE_BEGIN(MSG_ALL, SVC_TEMPENTITY);
	WRITE_BYTE(TE_BEAMPOINTS);
	WRITE_COORD(bbox_max.x); WRITE_COORD(bbox_min.y); WRITE_COORD(bbox_min.z);
	WRITE_COORD(bbox_max.x); WRITE_COORD(bbox_min.y); WRITE_COORD(bbox_max.z);
	WRITE_SHORT(g_sModelIndexLaser);
	WRITE_BYTE(0); WRITE_BYTE(0); WRITE_BYTE(10); WRITE_BYTE(5); WRITE_BYTE(0);
	WRITE_BYTE(255); WRITE_BYTE(0); WRITE_BYTE(0); WRITE_BYTE(255); WRITE_BYTE(0);
	MESSAGE_END();

	MESSAGE_BEGIN(MSG_ALL, SVC_TEMPENTITY);
	WRITE_BYTE(TE_BEAMPOINTS);
	WRITE_COORD(bbox_min.x); WRITE_COORD(bbox_max.y); WRITE_COORD(bbox_min.z);
	WRITE_COORD(bbox_min.x); WRITE_COORD(bbox_max.y); WRITE_COORD(bbox_max.z);
	WRITE_SHORT(g_sModelIndexLaser);
	WRITE_BYTE(0); WRITE_BYTE(0); WRITE_BYTE(10); WRITE_BYTE(5); WRITE_BYTE(0);
	WRITE_BYTE(255); WRITE_BYTE(0); WRITE_BYTE(0); WRITE_BYTE(255); WRITE_BYTE(0);
	MESSAGE_END();

	MESSAGE_BEGIN(MSG_ALL, SVC_TEMPENTITY);
	WRITE_BYTE(TE_BEAMPOINTS);
	WRITE_COORD(bbox_max.x); WRITE_COORD(bbox_max.y); WRITE_COORD(bbox_min.z);
	WRITE_COORD(bbox_max.x); WRITE_COORD(bbox_max.y); WRITE_COORD(bbox_max.z);
	WRITE_SHORT(g_sModelIndexLaser);
	WRITE_BYTE(0); WRITE_BYTE(0); WRITE_BYTE(10); WRITE_BYTE(5); WRITE_BYTE(0);
	WRITE_BYTE(255); WRITE_BYTE(0); WRITE_BYTE(0); WRITE_BYTE(255); WRITE_BYTE(0);
	MESSAGE_END();

	// 2. НИЖНЯЯ ГРАНЬ - ЗЕЛЕНАЯ
	MESSAGE_BEGIN(MSG_ALL, SVC_TEMPENTITY);
	WRITE_BYTE(TE_BEAMPOINTS);
	WRITE_COORD(bbox_min.x); WRITE_COORD(bbox_min.y); WRITE_COORD(bbox_min.z);
	WRITE_COORD(bbox_max.x); WRITE_COORD(bbox_min.y); WRITE_COORD(bbox_min.z);
	WRITE_SHORT(g_sModelIndexLaser);
	WRITE_BYTE(0); WRITE_BYTE(0); WRITE_BYTE(10); WRITE_BYTE(5); WRITE_BYTE(0);
	WRITE_BYTE(0); WRITE_BYTE(255); WRITE_BYTE(0); WRITE_BYTE(255); WRITE_BYTE(0);
	MESSAGE_END();

	MESSAGE_BEGIN(MSG_ALL, SVC_TEMPENTITY);
	WRITE_BYTE(TE_BEAMPOINTS);
	WRITE_COORD(bbox_min.x); WRITE_COORD(bbox_min.y); WRITE_COORD(bbox_min.z);
	WRITE_COORD(bbox_min.x); WRITE_COORD(bbox_max.y); WRITE_COORD(bbox_min.z);
	WRITE_SHORT(g_sModelIndexLaser);
	WRITE_BYTE(0); WRITE_BYTE(0); WRITE_BYTE(10); WRITE_BYTE(5); WRITE_BYTE(0);
	WRITE_BYTE(0); WRITE_BYTE(255); WRITE_BYTE(0); WRITE_BYTE(255); WRITE_BYTE(0);
	MESSAGE_END();

	MESSAGE_BEGIN(MSG_ALL, SVC_TEMPENTITY);
	WRITE_BYTE(TE_BEAMPOINTS);
	WRITE_COORD(bbox_max.x); WRITE_COORD(bbox_max.y); WRITE_COORD(bbox_min.z);
	WRITE_COORD(bbox_max.x); WRITE_COORD(bbox_min.y); WRITE_COORD(bbox_min.z);
	WRITE_SHORT(g_sModelIndexLaser);
	WRITE_BYTE(0); WRITE_BYTE(0); WRITE_BYTE(10); WRITE_BYTE(5); WRITE_BYTE(0);
	WRITE_BYTE(0); WRITE_BYTE(255); WRITE_BYTE(0); WRITE_BYTE(255); WRITE_BYTE(0);
	MESSAGE_END();

	MESSAGE_BEGIN(MSG_ALL, SVC_TEMPENTITY);
	WRITE_BYTE(TE_BEAMPOINTS);
	WRITE_COORD(bbox_max.x); WRITE_COORD(bbox_max.y); WRITE_COORD(bbox_min.z);
	WRITE_COORD(bbox_min.x); WRITE_COORD(bbox_max.y); WRITE_COORD(bbox_min.z);
	WRITE_SHORT(g_sModelIndexLaser);
	WRITE_BYTE(0); WRITE_BYTE(0); WRITE_BYTE(10); WRITE_BYTE(5); WRITE_BYTE(0);
	WRITE_BYTE(0); WRITE_BYTE(255); WRITE_BYTE(0); WRITE_BYTE(255); WRITE_BYTE(0);
	MESSAGE_END();

	// 3. ВЕРХНЯЯ ГРАНЬ - СИНЯЯ
	MESSAGE_BEGIN(MSG_ALL, SVC_TEMPENTITY);
	WRITE_BYTE(TE_BEAMPOINTS);
	WRITE_COORD(bbox_min.x); WRITE_COORD(bbox_min.y); WRITE_COORD(bbox_max.z);
	WRITE_COORD(bbox_max.x); WRITE_COORD(bbox_min.y); WRITE_COORD(bbox_max.z);
	WRITE_SHORT(g_sModelIndexLaser);
	WRITE_BYTE(0); WRITE_BYTE(0); WRITE_BYTE(10); WRITE_BYTE(5); WRITE_BYTE(0);
	WRITE_BYTE(0); WRITE_BYTE(0); WRITE_BYTE(255); WRITE_BYTE(255); WRITE_BYTE(0);
	MESSAGE_END();

	MESSAGE_BEGIN(MSG_ALL, SVC_TEMPENTITY);
	WRITE_BYTE(TE_BEAMPOINTS);
	WRITE_COORD(bbox_min.x); WRITE_COORD(bbox_min.y); WRITE_COORD(bbox_max.z);
	WRITE_COORD(bbox_min.x); WRITE_COORD(bbox_max.y); WRITE_COORD(bbox_max.z);
	WRITE_SHORT(g_sModelIndexLaser);
	WRITE_BYTE(0); WRITE_BYTE(0); WRITE_BYTE(10); WRITE_BYTE(5); WRITE_BYTE(0);
	WRITE_BYTE(0); WRITE_BYTE(0); WRITE_BYTE(255); WRITE_BYTE(255); WRITE_BYTE(0);
	MESSAGE_END();

	MESSAGE_BEGIN(MSG_ALL, SVC_TEMPENTITY);
	WRITE_BYTE(TE_BEAMPOINTS);
	WRITE_COORD(bbox_max.x); WRITE_COORD(bbox_max.y); WRITE_COORD(bbox_max.z);
	WRITE_COORD(bbox_max.x); WRITE_COORD(bbox_min.y); WRITE_COORD(bbox_max.z);
	WRITE_SHORT(g_sModelIndexLaser);
	WRITE_BYTE(0); WRITE_BYTE(0); WRITE_BYTE(10); WRITE_BYTE(5); WRITE_BYTE(0);
	WRITE_BYTE(0); WRITE_BYTE(0); WRITE_BYTE(255); WRITE_BYTE(255); WRITE_BYTE(0);
	MESSAGE_END();

	MESSAGE_BEGIN(MSG_ALL, SVC_TEMPENTITY);
	WRITE_BYTE(TE_BEAMPOINTS);
	WRITE_COORD(bbox_max.x); WRITE_COORD(bbox_max.y); WRITE_COORD(bbox_max.z);
	WRITE_COORD(bbox_min.x); WRITE_COORD(bbox_max.y); WRITE_COORD(bbox_max.z);
	WRITE_SHORT(g_sModelIndexLaser);
	WRITE_BYTE(0); WRITE_BYTE(0); WRITE_BYTE(10); WRITE_BYTE(5); WRITE_BYTE(0);
	WRITE_BYTE(0); WRITE_BYTE(0); WRITE_BYTE(255); WRITE_BYTE(255); WRITE_BYTE(0);
	MESSAGE_END();
}





















































void Assassin_Move(void)
{
	if (!g_pAssassin) return;
	if (g_bAssassinDeathSequence) return;

	CHAssassin* pAssassin = (CHAssassin*)g_pAssassin;



	if (pAssassin->pev->movetype != MOVETYPE_FLY)
	{
		pAssassin->pev->solid = SOLID_SLIDEBOX;
		pAssassin->pev->movetype = MOVETYPE_STEP;
		Assassin_MoveWithJerk();

		// ★★★★ ВСЕГДА ПРОВЕРЯЕМ ДВЕРИ ПРИ ДВИЖЕНИИ ★★★★
		Assassin_CheckObjectInteraction();
		Assassin_PhysicsInteraction();
	}






	// ★★★★ ОТЛАДКА: ПРОВЕРКА ЧТО ФУНКЦИЯ ВЫЗЫВАЕТСЯ ★★★★
	static int moveCount = 0;
	moveCount++;
	if (moveCount % 50 == 0) // Выводим каждые 50 вызовов
	{
		ALERT(at_console, "Assassin_Move called %d times\n", moveCount);
	}


	// ★★★★ ПОСТОЯННАЯ ОТРИСОВКА BBOX ЕСЛИ ВКЛЮЧЕНА ★★★★
	if (g_bDrawAssassinBBox)
	{
		if (gpGlobals->time - g_flLastBBoxDraw > g_flBBoxDrawInterval)
		{
			DebugDrawAssassinFullBBox();
			g_flLastBBoxDraw = gpGlobals->time;
		}
	}

	if (pAssassin->pev->movetype != MOVETYPE_FLY)
	{
		pAssassin->pev->solid = SOLID_SLIDEBOX;
		pAssassin->pev->movetype = MOVETYPE_STEP;
		Assassin_MoveWithJerk();

		Assassin_CheckObjectInteraction();  // ← Проверка взаимодействия с объектами
		Assassin_PhysicsInteraction();      // ← Проверка физических столкновений
	}
}


void Assassin_StopMove(void)
{
	//ALERT(at_console, "Assassin_StopMove CALLED\n");

	if (!g_pAssassin) return;

	CHAssassin* pAssassin = (CHAssassin*)g_pAssassin;

	// ПРОСТАЯ ОСТАНОВКА
	pAssassin->pev->velocity.x = 0;
	pAssassin->pev->velocity.y = 0;

	if (pAssassin->pev->flags & FL_ONGROUND)
	{
		pAssassin->SetActivity(ACT_IDLE);
	}

	//ALERT(at_console, "Movement stopped completely\n");
}


void Assassin_AirControl(Vector moveDirection)
{
	if (!g_pAssassin) return;
	CHAssassin* pAssassin = (CHAssassin*)g_pAssassin;

	if (pAssassin->pev->flags & FL_ONGROUND) return;

	// Сохраняем вертикальную скорость
	float savedVelocityZ = pAssassin->pev->velocity.z;

	// Преобразуем направление
	UTIL_MakeVectors(pAssassin->pev->angles);
	Vector forward = gpGlobals->v_forward;
	Vector right = gpGlobals->v_right;

	Vector worldMoveDirection = (forward * moveDirection.x) + (right * moveDirection.y);
	worldMoveDirection = worldMoveDirection.Normalize();

	// Более мягкое воздушное управление
	float airControlPower = 150.0f; // Уменьшена мощность
	float inertiaFactor = 0.15f;    // Уменьшена отзывчивость
	float maxAirSpeed = 300.0f;     // Уменьшена максимальная скорость

	Vector desiredVelocity = worldMoveDirection * airControlPower;
	desiredVelocity.z = 0;

	// Плавное изменение скорости
	pAssassin->pev->velocity.x = pAssassin->pev->velocity.x * (1.0f - inertiaFactor) + desiredVelocity.x * inertiaFactor;
	pAssassin->pev->velocity.y = pAssassin->pev->velocity.y * (1.0f - inertiaFactor) + desiredVelocity.y * inertiaFactor;

	// Восстанавливаем вертикальную скорость
	pAssassin->pev->velocity.z = savedVelocityZ;

	// Ограничение скорости
	float horizontalSpeed = sqrt(pAssassin->pev->velocity.x * pAssassin->pev->velocity.x +
		pAssassin->pev->velocity.y * pAssassin->pev->velocity.y);

	if (horizontalSpeed > maxAirSpeed) {
		float scale = maxAirSpeed / horizontalSpeed;
		pAssassin->pev->velocity.x *= scale;
		pAssassin->pev->velocity.y *= scale;
	}
}






void Assassin_SetSequence(int sequence)
{
	if (!g_pAssassin) return;

	if (!FStrEq(STRING(g_pAssassin->pev->classname), "monster_human_assassin"))
		return;

	CHAssassin* pAssassin = (CHAssassin*)g_pAssassin;
	pAssassin->pev->sequence = sequence;
	pAssassin->pev->frame = 0;
	pAssassin->ResetSequenceInfo();
}

int Assassin_LookupActivity(int activity)
{
	if (!g_pAssassin) return -1;

	if (!FStrEq(STRING(g_pAssassin->pev->classname), "monster_human_assassin"))
		return -1;

	CHAssassin* pAssassin = (CHAssassin*)g_pAssassin;
	return pAssassin->LookupActivity(activity);
}

bool Assassin_CanSeeEnemy(void)
{
	if (!g_pAssassin)
	{
		//ALERT(at_console, "Assassin_CanSeeEnemy: No assassin\n");
		return false;
	}

	if (!FStrEq(STRING(g_pAssassin->pev->classname), "monster_human_assassin"))
	{
		//ALERT(at_console, "Assassin_CanSeeEnemy: Not an assassin\n");
		return false;
	}

	CHAssassin* pAssassin = (CHAssassin*)g_pAssassin;

	if (!pAssassin->m_hEnemy)
	{
		//ALERT(at_console, "Assassin_CanSeeEnemy: No enemy\n");
		return false;
	}

	bool canSee = (pAssassin->HasConditions(bits_COND_SEE_ENEMY));
	//ALERT(at_console, "Assassin_CanSeeEnemy: %d\n", canSee);

	return canSee;
}


void Assassin_PlayLandAnimation(void)
{
	if (!g_pAssassin) return;

	CHAssassin* pAssassin = (CHAssassin*)g_pAssassin;

	// НЕ меняем размеры - они должны оставаться стандартными
	// Просто возвращаем нормальную физику
	pAssassin->pev->movetype = MOVETYPE_STEP;

	// Анимация приземления
	int landSequence = pAssassin->LookupSequence("land");
	if (landSequence == -1)
		landSequence = pAssassin->LookupSequence("crouch");

	if (landSequence != -1)
	{
		pAssassin->pev->sequence = landSequence;
		pAssassin->pev->frame = 0;
		pAssassin->ResetSequenceInfo();
	}

	// Звук приземления
	EMIT_SOUND(pAssassin->edict(), CHAN_BODY, "player/pl_step2.wav", 0.6, ATTN_NORM);

	//ALERT(at_console, "Landed - restored normal physics\n");
}


// Глобальные функции для работы с анимациями ассасина
void Assassin_PlayJumpSound(void)
{
	if (!g_pAssassin) return;

	// СЛУЧАЙНЫЙ ЗВУК ШАГА для прыжка
	switch (RANDOM_LONG(0, 3))
	{
	case 0:
		EMIT_SOUND(g_pAssassin->edict(), CHAN_BODY, "player/pl_step1.wav", 0.7, ATTN_NORM);
		break;
	case 1:
		EMIT_SOUND(g_pAssassin->edict(), CHAN_BODY, "player/pl_step3.wav", 0.7, ATTN_NORM);
		break;
	case 2:
		EMIT_SOUND(g_pAssassin->edict(), CHAN_BODY, "player/pl_step2.wav", 0.7, ATTN_NORM);
		break;
	case 3:
		EMIT_SOUND(g_pAssassin->edict(), CHAN_BODY, "player/pl_step4.wav", 0.7, ATTN_NORM);
		break;
	}
}


void Assassin_PlayLandSound(void)
{
	if (!g_pAssassin) return;

	// ЗВУК ПРИЗЕМЛЕНИЯ
	EMIT_SOUND(g_pAssassin->edict(), CHAN_BODY, "player/pl_step2.wav", 0.5, ATTN_NORM);
}


void Assassin_SetAnimation(const char* animationName)
{
	if (!g_pAssassin) return;

	if (!FStrEq(STRING(g_pAssassin->pev->classname), "monster_human_assassin"))
		return;

	CHAssassin* pAssassin = (CHAssassin*)g_pAssassin;

	int sequence = pAssassin->LookupSequence(animationName);
	if (sequence != -1)
	{
		pAssassin->pev->sequence = sequence;
		pAssassin->pev->frame = 0;
		pAssassin->ResetSequenceInfo();
		//ALERT(at_console, "Animation set to: %s (seq: %d)\n", animationName, sequence);
	}
	else
	{
		//ALERT(at_console, "ERROR: Animation %s not found!\n", animationName);
	}
}


int Assassin_GetCurrentSequence(void)
{
	if (!g_pAssassin) return -1;
	return g_pAssassin->pev->sequence;
}

int Assassin_LookupSequence(const char* animationName)
{
	if (!g_pAssassin) return -1;

	if (!FStrEq(STRING(g_pAssassin->pev->classname), "monster_human_assassin"))
		return -1;

	CHAssassin* pAssassin = (CHAssassin*)g_pAssassin;
	return pAssassin->LookupSequence(animationName);
}




void Assassin_RestoreCollisions(void)
{
	if (!g_pAssassin) return;

	CHAssassin* pAssassin = (CHAssassin*)g_pAssassin;

	// Восстанавливаем коллизии и физику
	pAssassin->pev->solid = SOLID_SLIDEBOX;
	pAssassin->pev->movetype = MOVETYPE_STEP;

	//ALERT(at_console, "Collisions restored\n");
}























































TYPEDESCRIPTION	CHAssassin::m_SaveData[] = 
{
	DEFINE_FIELD( CHAssassin, m_flLastShot, FIELD_TIME ),
	DEFINE_FIELD( CHAssassin, m_flDiviation, FIELD_FLOAT ),

	DEFINE_FIELD( CHAssassin, m_flNextJump, FIELD_TIME ),
	DEFINE_FIELD( CHAssassin, m_vecJumpVelocity, FIELD_VECTOR ),

	DEFINE_FIELD( CHAssassin, m_flNextGrenadeCheck, FIELD_TIME ),
	DEFINE_FIELD( CHAssassin, m_vecTossVelocity, FIELD_VECTOR ),
	DEFINE_FIELD( CHAssassin, m_fThrowGrenade, FIELD_BOOLEAN ),

	DEFINE_FIELD( CHAssassin, m_iTargetRanderamt, FIELD_INTEGER ),
	DEFINE_FIELD( CHAssassin, m_iFrustration, FIELD_INTEGER ),
};

IMPLEMENT_SAVERESTORE( CHAssassin, CBaseMonster );


//=========================================================
// DieSound
//=========================================================
void CHAssassin :: DeathSound ( void )
{
}

//=========================================================
// IdleSound
//=========================================================
void CHAssassin :: IdleSound ( void )
{
}

//=========================================================
// ISoundMask - returns a bit mask indicating which types
// of sounds this monster regards. 
//=========================================================
int CHAssassin :: ISoundMask ( void) 
{
	return	bits_SOUND_WORLD	|
			bits_SOUND_COMBAT	|
			bits_SOUND_DANGER	|
			bits_SOUND_PLAYER;
}


//=========================================================
// Classify - indicates this monster's place in the 
// relationship table.
//=========================================================
int	CHAssassin :: Classify ( void )
{
	if (IsBeingControlled())
	{
		//ALERT(at_console, "Assassin is controlled - setting to CLASS_NONE\n");
		return CLASS_NONE;  // Нейтральный когда управляем
	}
	else
	{
		//ALERT(at_console, "Assassin is AI - setting to CLASS_HUMAN_MILITARY\n");
		return CLASS_HUMAN_MILITARY;  // Обычный враг когда не управляем
	}
}

//=========================================================
// SetYawSpeed - allows each sequence to have a different
// turn rate associated with it.
//=========================================================
void CHAssassin :: SetYawSpeed ( void )
{
	int ys;

	switch ( m_Activity )
	{
	case ACT_TURN_LEFT:
	case ACT_TURN_RIGHT:
		ys = 360;
		break;
	default:			
		ys = 360;
		break;
	}

	pev->yaw_speed = ys;
}


//=========================================================
// Shoot
//=========================================================
void CHAssassin::Shoot(void)
{
	Vector vecShootDir;
	Vector vecShootOrigin = GetGunPosition();

	if (IsBeingControlled())
	{

		Vector fullAngles = pev->angles;
		fullAngles.x = m_flPlayerWishPitch;

		UTIL_MakeVectors(fullAngles);
		vecShootDir = gpGlobals->v_forward;

		// Запускаем анимацию только при первом выстреле
		if (m_Activity != ACT_RANGE_ATTACK1)
		{
			SetActivity(ACT_RANGE_ATTACK1);
		}
	}
	else
	{
		if (m_hEnemy == NULL)
			return;
		vecShootDir = ShootAtEnemy(vecShootOrigin);
	}

	// Разброс
	if (m_flLastShot + 2 < gpGlobals->time)
		m_flDiviation = 0.10;
	else
	{
		m_flDiviation -= 0.01;
		if (m_flDiviation < 0.02)
			m_flDiviation = 0.02;
	}
	m_flLastShot = gpGlobals->time;

	UTIL_MakeVectors(pev->angles);
	Vector vecShellVelocity = gpGlobals->v_right * RANDOM_FLOAT(40, 90) + gpGlobals->v_up * RANDOM_FLOAT(75, 200) + gpGlobals->v_forward * RANDOM_FLOAT(-40, 40);
	EjectBrass(pev->origin + gpGlobals->v_up * 32 + gpGlobals->v_forward * 12, vecShellVelocity, pev->angles.y, m_iShell, TE_BOUNCE_SHELL);
	FireBullets(1, vecShootOrigin, vecShootDir, Vector(m_flDiviation, m_flDiviation, m_flDiviation), 2048, BULLET_MONSTER_9MM);

	switch (RANDOM_LONG(0, 1))
	{
	case 0: EMIT_SOUND(ENT(pev), CHAN_WEAPON, "weapons/pl_gun1.wav", RANDOM_FLOAT(0.6, 0.8), ATTN_NORM); break;
	case 1: EMIT_SOUND(ENT(pev), CHAN_WEAPON, "weapons/pl_gun2.wav", RANDOM_FLOAT(0.6, 0.8), ATTN_NORM); break;
	}

	pev->effects |= EF_MUZZLEFLASH;
	Vector angDir = UTIL_VecToAngles(vecShootDir);
	SetBlending(0, angDir.x);
	m_cAmmoLoaded--;
}


//=========================================================
// HandleAnimEvent - catches the monster-specific messages
// that occur when tagged animation frames are played.
//
// Returns number of events handled, 0 if none.
//=========================================================
void CHAssassin :: HandleAnimEvent( MonsterEvent_t *pEvent )
{
	switch( pEvent->event )
	{
	case ASSASSIN_AE_SHOOT1:

		if (!IsBeingControlled())
		{
			Shoot();
		}

		break;
	case ASSASSIN_AE_TOSS1:
		{
			UTIL_MakeVectors( pev->angles );
			CGrenade::ShootTimed( pev, pev->origin + gpGlobals->v_forward * 34 + Vector (0, 0, 32), m_vecTossVelocity, 2.0 );

			m_flNextGrenadeCheck = gpGlobals->time + 6;// wait six seconds before even looking again to see if a grenade can be thrown.
			m_fThrowGrenade = FALSE;
			// !!!LATER - when in a group, only try to throw grenade if ordered.
		}
		break;
	case ASSASSIN_AE_JUMP:
		{
			// ALERT( at_console, "jumping");
			UTIL_MakeAimVectors( pev->angles );
			pev->movetype = MOVETYPE_TOSS;
			pev->flags &= ~FL_ONGROUND;
			pev->velocity = m_vecJumpVelocity;
			m_flNextJump = gpGlobals->time + 3.0;
		}
		break;
	case ASSASSIN_AE_MELEE1:
	case ASSASSIN_AE_MELEE2:
		// Урон уже нанесён в PerformMeleeAttack
		break;




	default:
		CBaseMonster::HandleAnimEvent( pEvent );
		break;
	}
}

//=========================================================
// Spawn
//=========================================================
void CHAssassin :: Spawn()
{
	Precache( );

	SET_MODEL(ENT(pev), "models/hassassin.mdl");
	UTIL_SetSize(pev, VEC_HUMAN_HULL_MIN, VEC_HUMAN_HULL_MAX);

	pev->solid			= SOLID_SLIDEBOX;
	pev->movetype		= MOVETYPE_STEP;
	m_bloodColor		= BLOOD_COLOR_RED;
	pev->effects		= 0;
	pev->health			= gSkillData.hassassinHealth;
	m_flFieldOfView		= VIEW_FIELD_WIDE; // indicates the width of this monster's forward view cone ( as a dotproduct result )
	m_MonsterState		= MONSTERSTATE_NONE;
	m_afCapability		= bits_CAP_MELEE_ATTACK1 | bits_CAP_DOORS_GROUP;
	pev->friction		= 1;

	m_HackedGunPos		= Vector( 0, 24, 48 );

	m_iTargetRanderamt	= 20;
	pev->renderamt		= 20;
	pev->rendermode		= kRenderTransTexture;


	pev->flags |= FL_CLIENT;           // Как игрок (важно для дверей!)
	pev->flags &= ~FL_MONSTER;         // Убираем флаг монстра
	pev->flags |= FL_ONGROUND;         // Для проверок



	// ★★★★ ИНИЦИАЛИЗАЦИЯ УПРАВЛЕНИЯ ★★★★
	m_flLastPlayerInputTime = 0;
	m_flNextDebugTime = 0;
	
	m_bPlayerWalking = FALSE;
	m_bIsSliding = FALSE;

	m_flPlayerWishPitch = 0;
	m_bPlayerWishAltAttack = FALSE; 
	m_iLastAltAttackType = 0;
	m_flNextMeleeAttack = 0;

	MonsterInit();
}

//=========================================================
// Precache - precaches all resources this monster needs
//=========================================================
void CHAssassin :: Precache()
{
	PRECACHE_MODEL("models/hassassin.mdl");

	PRECACHE_SOUND("weapons/pl_gun1.wav");
	PRECACHE_SOUND("weapons/pl_gun2.wav");

	PRECACHE_SOUND("debris/beamstart1.wav");

	m_iShell = PRECACHE_MODEL ("models/shell.mdl");// brass shell




		// ★★★★ ДОБАВИТЬ ЗВУКИ ИСПОЛЬЗОВАНИЯ ★★★★
	PRECACHE_SOUND("common/wpn_select.wav");      // Звук успеха
	PRECACHE_SOUND("common/wpn_denyselect.wav");  // Звук отказа

	PRECACHE_SOUND("buttons/button1.wav");        // Звук кнопки





}	
	


//=========================================================
// AI Schedules Specific to this monster
//=========================================================

//=========================================================
// Fail Schedule
//=========================================================
Task_t	tlAssassinFail[] =
{
	{ TASK_STOP_MOVING,			0				},
	{ TASK_SET_ACTIVITY,		(float)ACT_IDLE },
	{ TASK_WAIT_FACE_ENEMY,		(float)2		},
	// { TASK_WAIT_PVS,			(float)0		},
	{ TASK_SET_SCHEDULE,		(float)SCHED_CHASE_ENEMY },
};

Schedule_t	slAssassinFail[] =
{
	{
		tlAssassinFail,
		ARRAYSIZE ( tlAssassinFail ),
		bits_COND_LIGHT_DAMAGE		|
		bits_COND_HEAVY_DAMAGE		|
		bits_COND_PROVOKED			|
		bits_COND_CAN_RANGE_ATTACK1 |
		bits_COND_CAN_RANGE_ATTACK2 |
		bits_COND_CAN_MELEE_ATTACK1 |
		bits_COND_HEAR_SOUND,
	
		bits_SOUND_DANGER |
		bits_SOUND_PLAYER,
		"AssassinFail"
	},
};


//=========================================================
// Enemy exposed Agrunt's cover
//=========================================================
Task_t	tlAssassinExposed[] =
{
	{ TASK_STOP_MOVING,			(float)0							},
	{ TASK_RANGE_ATTACK1,		(float)0							},
	{ TASK_SET_FAIL_SCHEDULE,	(float)SCHED_ASSASSIN_JUMP			},
	{ TASK_SET_SCHEDULE,		(float)SCHED_TAKE_COVER_FROM_ENEMY	},
};

Schedule_t slAssassinExposed[] =
{
	{
		tlAssassinExposed,
		ARRAYSIZE ( tlAssassinExposed ),
		bits_COND_CAN_MELEE_ATTACK1,
		0,
		"AssassinExposed",
	},
};


//=========================================================
// Take cover from enemy! Tries lateral cover before node 
// cover! 
//=========================================================
Task_t	tlAssassinTakeCoverFromEnemy[] =
{
	{ TASK_STOP_MOVING,				(float)0					},
	{ TASK_WAIT,					(float)0.2					},
	{ TASK_SET_FAIL_SCHEDULE,		(float)SCHED_RANGE_ATTACK1	},
	{ TASK_FIND_COVER_FROM_ENEMY,	(float)0					},
	{ TASK_RUN_PATH,				(float)0					},
	{ TASK_WAIT_FOR_MOVEMENT,		(float)0					},
	{ TASK_REMEMBER,				(float)bits_MEMORY_INCOVER	},
	{ TASK_FACE_ENEMY,				(float)0					},
};

Schedule_t	slAssassinTakeCoverFromEnemy[] =
{
	{ 
		tlAssassinTakeCoverFromEnemy,
		ARRAYSIZE ( tlAssassinTakeCoverFromEnemy ), 
		bits_COND_NEW_ENEMY |
		bits_COND_CAN_MELEE_ATTACK1		|
		bits_COND_HEAR_SOUND,
		
		bits_SOUND_DANGER,
		"AssassinTakeCoverFromEnemy"
	},
};


//=========================================================
// Take cover from enemy! Tries lateral cover before node 
// cover! 
//=========================================================
Task_t	tlAssassinTakeCoverFromEnemy2[] =
{
	{ TASK_STOP_MOVING,				(float)0					},
	{ TASK_WAIT,					(float)0.2					},
	{ TASK_FACE_ENEMY,				(float)0					},
	{ TASK_RANGE_ATTACK1,			(float)0					},
	{ TASK_SET_FAIL_SCHEDULE,		(float)SCHED_RANGE_ATTACK2	},
	{ TASK_FIND_FAR_NODE_COVER_FROM_ENEMY,	(float)384			},
	{ TASK_RUN_PATH,				(float)0					},
	{ TASK_WAIT_FOR_MOVEMENT,		(float)0					},
	{ TASK_REMEMBER,				(float)bits_MEMORY_INCOVER	},
	{ TASK_FACE_ENEMY,				(float)0					},
};

Schedule_t	slAssassinTakeCoverFromEnemy2[] =
{
	{ 
		tlAssassinTakeCoverFromEnemy2,
		ARRAYSIZE ( tlAssassinTakeCoverFromEnemy2 ), 
		bits_COND_NEW_ENEMY |
		bits_COND_CAN_MELEE_ATTACK2		|
		bits_COND_HEAR_SOUND,
		
		bits_SOUND_DANGER,
		"AssassinTakeCoverFromEnemy2"
	},
};


//=========================================================
// hide from the loudest sound source
//=========================================================
Task_t	tlAssassinTakeCoverFromBestSound[] =
{
	{ TASK_SET_FAIL_SCHEDULE,			(float)SCHED_MELEE_ATTACK1	},
	{ TASK_STOP_MOVING,					(float)0					},
	{ TASK_FIND_COVER_FROM_BEST_SOUND,	(float)0					},
	{ TASK_RUN_PATH,					(float)0					},
	{ TASK_WAIT_FOR_MOVEMENT,			(float)0					},
	{ TASK_REMEMBER,					(float)bits_MEMORY_INCOVER	},
	{ TASK_TURN_LEFT,					(float)179					},
};

Schedule_t	slAssassinTakeCoverFromBestSound[] =
{
	{ 
		tlAssassinTakeCoverFromBestSound,
		ARRAYSIZE ( tlAssassinTakeCoverFromBestSound ), 
		bits_COND_NEW_ENEMY,
		0,
		"AssassinTakeCoverFromBestSound"
	},
};





//=========================================================
// AlertIdle Schedules
//=========================================================
Task_t	tlAssassinHide[] =
{
	{ TASK_STOP_MOVING,			0						 },
	{ TASK_SET_ACTIVITY,		(float)ACT_IDLE			 },
	{ TASK_WAIT,				(float)2				 },
	{ TASK_SET_SCHEDULE,		(float)SCHED_CHASE_ENEMY },
};

Schedule_t	slAssassinHide[] =
{
	{ 
		tlAssassinHide,
		ARRAYSIZE ( tlAssassinHide ), 
		bits_COND_NEW_ENEMY				|
		bits_COND_SEE_ENEMY				|
		bits_COND_SEE_FEAR				|
		bits_COND_LIGHT_DAMAGE			|
		bits_COND_HEAVY_DAMAGE			|
		bits_COND_PROVOKED		|
		bits_COND_HEAR_SOUND,
		
		bits_SOUND_DANGER,
		"AssassinHide"
	},
};



//=========================================================
// HUNT Schedules
//=========================================================
Task_t tlAssassinHunt[] = 
{
	{ TASK_GET_PATH_TO_ENEMY,	(float)0		},
	{ TASK_RUN_PATH,			(float)0		},
	{ TASK_WAIT_FOR_MOVEMENT,	(float)0		},
};

Schedule_t slAssassinHunt[] =
{
	{ 
		tlAssassinHunt,
		ARRAYSIZE ( tlAssassinHunt ),
		bits_COND_NEW_ENEMY			|
		// bits_COND_SEE_ENEMY			|
		bits_COND_CAN_RANGE_ATTACK1	|
		bits_COND_HEAR_SOUND,
		
		bits_SOUND_DANGER,
		"AssassinHunt"
	},
};


//=========================================================
// Jumping Schedules
//=========================================================
Task_t	tlAssassinJump[] =
{
	{ TASK_STOP_MOVING,			(float)0		},
	{ TASK_PLAY_SEQUENCE,		(float)ACT_HOP	},
	{ TASK_SET_SCHEDULE,		(float)SCHED_ASSASSIN_JUMP_ATTACK },
};

Schedule_t	slAssassinJump[] =
{
	{ 
		tlAssassinJump,
		ARRAYSIZE ( tlAssassinJump ), 
		0, 
		0, 
		"AssassinJump"
	},
};


//=========================================================
// repel 
//=========================================================
Task_t	tlAssassinJumpAttack[] =
{
	{ TASK_SET_FAIL_SCHEDULE,	(float)SCHED_ASSASSIN_JUMP_LAND	},
	// { TASK_SET_ACTIVITY,		(float)ACT_FLY	},
	{ TASK_ASSASSIN_FALL_TO_GROUND, (float)0		},
};


Schedule_t	slAssassinJumpAttack[] =
{
	{ 
		tlAssassinJumpAttack,
		ARRAYSIZE ( tlAssassinJumpAttack ), 
		0, 
		0,
		"AssassinJumpAttack"
	},
};


//=========================================================
// repel 
//=========================================================
Task_t	tlAssassinJumpLand[] =
{
	{ TASK_SET_FAIL_SCHEDULE,		(float)SCHED_ASSASSIN_EXPOSED	},
	// { TASK_SET_FAIL_SCHEDULE,		(float)SCHED_MELEE_ATTACK1	},
	{ TASK_SET_ACTIVITY,			(float)ACT_IDLE				},
	{ TASK_REMEMBER,				(float)bits_MEMORY_BADJUMP	},
	{ TASK_FIND_NODE_COVER_FROM_ENEMY,	(float)0					},
	{ TASK_RUN_PATH,				(float)0					},
	{ TASK_FORGET,					(float)bits_MEMORY_BADJUMP	},
	{ TASK_WAIT_FOR_MOVEMENT,		(float)0					},
	{ TASK_REMEMBER,				(float)bits_MEMORY_INCOVER	},
	{ TASK_FACE_ENEMY,				(float)0					},
	{ TASK_SET_FAIL_SCHEDULE,		(float)SCHED_RANGE_ATTACK1	},
};

Schedule_t	slAssassinJumpLand[] =
{
	{ 
		tlAssassinJumpLand,
		ARRAYSIZE ( tlAssassinJumpLand ), 
		0, 
		0,
		"AssassinJumpLand"
	},
};

DEFINE_CUSTOM_SCHEDULES( CHAssassin )
{
	slAssassinFail,
	slAssassinExposed,
	slAssassinTakeCoverFromEnemy,
	slAssassinTakeCoverFromEnemy2,
	slAssassinTakeCoverFromBestSound,
	slAssassinHide,
	slAssassinHunt,
	slAssassinJump,
	slAssassinJumpAttack,
	slAssassinJumpLand,
};

IMPLEMENT_CUSTOM_SCHEDULES( CHAssassin, CBaseMonster );


//=========================================================
// CheckMeleeAttack1 - jump like crazy if the enemy gets too close. 
//=========================================================
BOOL CHAssassin::CheckMeleeAttack1(float flDot, float flDist)
{
	// ★★★★ ПРИОРИТЕТ: УПРАВЛЕНИЕ ИГРОКОМ ★★★★
	if (IsBeingControlled())
	{
		// Прыжок обрабатывается в UpdatePlayerControl
		return FALSE;
	}

	// ★★★★ ВОССТАНАВЛИВАЕМ ОРИГИНАЛЬНУЮ ФИЗИКУ ПРИ ПРИЗЕМЛЕНИИ ★★★★
	if (IsBeingControlled() && (pev->flags & FL_ONGROUND) && pev->movetype == MOVETYPE_TOSS)
	{
		pev->movetype = MOVETYPE_STEP;
		ALERT(at_console, "AI: Landed and restored physics\n");
	}

	// Оригинальная логика AI (только для неуправляемого режима)
	if (m_flNextJump < gpGlobals->time &&
		(flDist <= 128 || HasMemory(bits_MEMORY_BADJUMP)) &&
		m_hEnemy != NULL)
	{
		TraceResult tr;
		Vector vecDest = pev->origin + Vector(RANDOM_FLOAT(-64, 64), RANDOM_FLOAT(-64, 64), 160);
		UTIL_TraceHull(pev->origin + Vector(0, 0, 36), vecDest + Vector(0, 0, 36),
			dont_ignore_monsters, human_hull, ENT(pev), &tr);

		if (tr.fStartSolid || tr.flFraction < 1.0)
			return FALSE;

		float flGravity = g_psv_gravity->value;
		float time = sqrt(160 / (0.5 * flGravity));
		float speed = flGravity * time / 160;
		m_vecJumpVelocity = (vecDest - pev->origin) * speed;

		return TRUE;
	}
	return FALSE;
}

//=========================================================
// CheckRangeAttack1  - drop a cap in their ass
//
//=========================================================
BOOL CHAssassin :: CheckRangeAttack1 ( float flDot, float flDist )
{


	// ★★★★ ОТКЛЮЧАЕМ AI СТРЕЛЬБУ ПРИ УПРАВЛЕНИИ ★★★★
	if (IsBeingControlled())
		return FALSE;




	// Оригинальная логика AI

	if ( !HasConditions( bits_COND_ENEMY_OCCLUDED ) && flDist > 64 && flDist <= 2048 /* && flDot >= 0.5 */ /* && NoFriendlyFire() */ )
	{
		TraceResult	tr;

		Vector vecSrc = GetGunPosition();

		// verify that a bullet fired from the gun will hit the enemy before the world.
		UTIL_TraceLine( vecSrc, m_hEnemy->BodyTarget(vecSrc), dont_ignore_monsters, ENT(pev), &tr);

		if ( tr.flFraction == 1 || tr.pHit == m_hEnemy->edict() )
		{
			return TRUE;
		}
	}
	return FALSE;
}

//=========================================================
// CheckRangeAttack2 - toss grenade is enemy gets in the way and is too close. 
//=========================================================
BOOL CHAssassin :: CheckRangeAttack2 ( float flDot, float flDist )
{


	if (IsBeingControlled())
		return FALSE; // Не атакуем в ближнем бою когда управляем




	m_fThrowGrenade = FALSE;
	if ( !FBitSet ( m_hEnemy->pev->flags, FL_ONGROUND ) )
	{
		// don't throw grenades at anything that isn't on the ground!
		return FALSE;
	}

	// don't get grenade happy unless the player starts to piss you off
	if ( m_iFrustration <= 2)
		return FALSE;

	if ( m_flNextGrenadeCheck < gpGlobals->time && !HasConditions( bits_COND_ENEMY_OCCLUDED ) && flDist <= 512 /* && flDot >= 0.5 */ /* && NoFriendlyFire() */ )
	{
		Vector vecToss = VecCheckThrow( pev, GetGunPosition( ), m_hEnemy->Center(), flDist, 0.5 ); // use dist as speed to get there in 1 second

		if ( vecToss != g_vecZero )
		{
			m_vecTossVelocity = vecToss;

			// throw a hand grenade
			m_fThrowGrenade = TRUE;

			return TRUE;
		}
	}

	return FALSE;
}


//=========================================================
// RunAI
//=========================================================
void CHAssassin::RunAI(void)
{
	// Приоритет: управление игроком
	if (IsBeingControlled())
	{
		UpdatePlayerControl();

		// Очищаем врага и условия
		if (m_hEnemy != NULL) {
			m_hEnemy = NULL;
		}
		ClearConditions(bits_COND_SEE_ENEMY | bits_COND_SEE_FEAR | bits_COND_SEE_DISLIKE);

		// Вызываем базовую логику для обработки движения
		CBaseMonster::RunAI();

		// Исправляем активность после базового RunAI
		if (PlayerWantsToMove()) {
			Activity correctActivity = m_bPlayerWalking ? ACT_WALK : ACT_RUN;
			if (m_Activity != correctActivity &&
				m_Activity != ACT_IDLE &&
				m_Activity != ACT_RANGE_ATTACK1 &&
				m_Activity != ACT_MELEE_ATTACK1 &&
				m_Activity != ACT_MELEE_ATTACK2)
			{
				SetActivity(correctActivity);
			}
		}

		// ★★★ УПРАВЛЕНИЕ ПРОЗРАЧНОСТЬЮ ★★★
		if (g_bInvisible) {
			Assassin_UpdateInvisibility();
		}
		else {
			// Оригинальное поведение прозрачности
			if (g_iSkillLevel != SKILL_HARD || m_hEnemy == NULL || pev->deadflag != DEAD_NO ||
				m_Activity == ACT_RUN || m_Activity == ACT_WALK || !(pev->flags & FL_ONGROUND))
				m_iTargetRanderamt = 255;
			else
				m_iTargetRanderamt = 20;

			if (pev->renderamt > m_iTargetRanderamt) {
				if (pev->renderamt == 255) {
					EMIT_SOUND(ENT(pev), CHAN_BODY, "debris/beamstart1.wav", 0.2, ATTN_NORM);
				}
				pev->renderamt = max(pev->renderamt - 50, m_iTargetRanderamt);
				pev->rendermode = kRenderTransTexture;
			}
			else if (pev->renderamt < m_iTargetRanderamt) {
				pev->renderamt = min(pev->renderamt + 50, m_iTargetRanderamt);
				if (pev->renderamt == 255)
					pev->rendermode = kRenderNormal;
			}
		}

		// ★★★★ ИСПРАВЛЕННЫЕ ЗВУКИ ШАГОВ ★★★★
		// Упрощённая проверка: персонаж на земле ИЛИ на лестнице
		BOOL bOnGround = (pev->flags & FL_ONGROUND) != 0;
		BOOL bOnLadder = (UTIL_PointContents(pev->origin + Vector(0, 0, 36)) == CONTENTS_LADDER);

		// Персонаж считается "на земле", если он на земле ИЛИ на лестнице
		BOOL bIsGrounded = bOnGround || bOnLadder;

		if (bIsGrounded && !g_bAssassinDeathSequence)
		{
			// Проверяем, движется ли персонаж
			float horizontalSpeed = sqrt(pev->velocity.x * pev->velocity.x +
				pev->velocity.y * pev->velocity.y);

			// Определяем режим движения
			BOOL bIsWalking = m_bPlayerWalking || (m_Activity == ACT_WALK);
			float speedThreshold = bIsWalking ? 5.0f : 10.0f;

			// Проверяем, есть ли движение
			BOOL bIsMoving = (horizontalSpeed > speedThreshold) ||
				(m_bAIMoving && (m_Activity == ACT_RUN || m_Activity == ACT_WALK)) ||
				(PlayerWantsToMove() && horizontalSpeed > 3.0f);

			if (bIsMoving)
			{
				if (bIsWalking) {
					// ХОДЬБА: медленные шаги
					static float lastWalkStepTime = 0;
					float walkStepInterval = 0.55f; // Интервал для ходьбы

					if (gpGlobals->time - lastWalkStepTime >= walkStepInterval) {
						lastWalkStepTime = gpGlobals->time;
						switch (RANDOM_LONG(0, 3)) {
						case 0: EMIT_SOUND(ENT(pev), CHAN_BODY, "player/pl_step1.wav", 0.5f, ATTN_NORM); break;
						case 1: EMIT_SOUND(ENT(pev), CHAN_BODY, "player/pl_step2.wav", 0.5f, ATTN_NORM); break;
						case 2: EMIT_SOUND(ENT(pev), CHAN_BODY, "player/pl_step3.wav", 0.5f, ATTN_NORM); break;
						case 3: EMIT_SOUND(ENT(pev), CHAN_BODY, "player/pl_step4.wav", 0.5f, ATTN_NORM); break;
						}
					}
				}
				else {
					// БЕГ: быстрые шаги (каждый второй кадр)
					static int iStep = 0;
					iStep = !iStep;
					if (iStep) {
						switch (RANDOM_LONG(0, 3)) {
						case 0: EMIT_SOUND(ENT(pev), CHAN_BODY, "player/pl_step1.wav", 0.8f, ATTN_NORM); break;
						case 1: EMIT_SOUND(ENT(pev), CHAN_BODY, "player/pl_step2.wav", 0.8f, ATTN_NORM); break;
						case 2: EMIT_SOUND(ENT(pev), CHAN_BODY, "player/pl_step3.wav", 0.8f, ATTN_NORM); break;
						case 3: EMIT_SOUND(ENT(pev), CHAN_BODY, "player/pl_step4.wav", 0.8f, ATTN_NORM); break;
						}
					}
				}
			}
		}

		// Отладка
		static float lastDebugTime = 0;
		if (gpGlobals->time - lastDebugTime > 1.0f) {
			float hSpeed = sqrt(pev->velocity.x * pev->velocity.x +
				pev->velocity.y * pev->velocity.y);
			BOOL bIsWalking = m_bPlayerWalking || (m_Activity == ACT_WALK);
			ALERT(at_console, "GROUND: %d, LADDER: %d, SPEED: %.1f, MODE: %s, Activity: %d, Invis: %d, Renderamt: %d\n",
				bOnGround ? 1 : 0,
				bOnLadder ? 1 : 0,
				hSpeed,
				bIsWalking ? "WALK" : "RUN",
				m_Activity,
				g_bInvisible ? 1 : 0,
				pev->renderamt);
			lastDebugTime = gpGlobals->time;
		}

		return;
	}


	// ★★★★ ОРИГИНАЛЬНАЯ ЛОГИКА AI ДЛЯ НЕУПРАВЛЯЕМОГО СОСТОЯНИЯ ★★★★
	CBaseMonster::RunAI();

	// always visible if moving
	if (g_iSkillLevel != SKILL_HARD || m_hEnemy == NULL || pev->deadflag != DEAD_NO ||
		m_Activity == ACT_RUN || m_Activity == ACT_WALK || !(pev->flags & FL_ONGROUND))
		m_iTargetRanderamt = 255;
	else
		m_iTargetRanderamt = 20;

	if (pev->renderamt > m_iTargetRanderamt)
	{
		if (pev->renderamt == 255)
		{
			EMIT_SOUND(ENT(pev), CHAN_BODY, "debris/beamstart1.wav", 0.2, ATTN_NORM);
		}
		pev->renderamt = max(pev->renderamt - 50, m_iTargetRanderamt);
		pev->rendermode = kRenderTransTexture;
	}
	else if (pev->renderamt < m_iTargetRanderamt)
	{
		pev->renderamt = min(pev->renderamt + 50, m_iTargetRanderamt);
		if (pev->renderamt == 255)
			pev->rendermode = kRenderNormal;
	}

	// ★★★★ ЗВУКИ ШАГОВ ДЛЯ AI-РЕЖИМА (НЕУПРАВЛЯЕМЫЙ) ★★★★
	if ((pev->flags & FL_ONGROUND) && (m_Activity == ACT_RUN || m_Activity == ACT_WALK))
	{
		static int iStep = 0;
		iStep = !iStep;
		if (iStep)
		{
			switch (RANDOM_LONG(0, 3))
			{
			case 0: EMIT_SOUND(ENT(pev), CHAN_BODY, "player/pl_step1.wav", 0.8, ATTN_NORM); break;
			case 1: EMIT_SOUND(ENT(pev), CHAN_BODY, "player/pl_step3.wav", 0.8, ATTN_NORM); break;
			case 2: EMIT_SOUND(ENT(pev), CHAN_BODY, "player/pl_step2.wav", 0.8, ATTN_NORM); break;
			case 3: EMIT_SOUND(ENT(pev), CHAN_BODY, "player/pl_step4.wav", 0.8, ATTN_NORM); break;
			}
		}
	}
}












//=========================================================
// StartTask
//=========================================================
void CHAssassin :: StartTask ( Task_t *pTask )
{
	switch ( pTask->iTask )
	{
	case TASK_RANGE_ATTACK2:
		if (!m_fThrowGrenade)
		{
			TaskComplete( );
		}
		else
		{
			CBaseMonster :: StartTask ( pTask );
		}
		break;
	case TASK_ASSASSIN_FALL_TO_GROUND:
		break;
	default:
		CBaseMonster :: StartTask ( pTask );
		break;
	}
}


//=========================================================
// RunTask 
//=========================================================
void CHAssassin::RunTask(Task_t* pTask)
{
	if (IsBeingControlled() && pTask->iTask == TASK_WAIT_FOR_MOVEMENT)
	{
		// Проверяем, не завершилось ли движение
		if (MovementIsComplete())
		{
			// Если игрок хочет двигаться, не завершаем задачу полностью
			if (PlayerWantsToMove())
			{
				// Просто обновляем состояние, но не вызываем TaskComplete
				m_bAIMoving = FALSE;
				return;
			}
			else
			{
				TaskComplete();
				return;
			}
		}
		return;
	}

	CBaseMonster::RunTask(pTask);
}

//=========================================================
// GetSchedule - Decides which type of schedule best suits
// the monster's current state and conditions. Then calls
// monster's member function to get a pointer to a schedule
// of the proper type.
//=========================================================
Schedule_t *CHAssassin :: GetSchedule ( void )
{


	// ★★★★ ПРИОРИТЕТ: УПРАВЛЕНИЕ ИГРОКОМ ★★★★
	if (IsBeingControlled())
	{
		// ★★★★ ПРОСТО ВОЗВРАЩАЕМ РАСПИСАНИЕ ПРЕСЛЕДОВАНИЯ ДЛЯ АНИМАЦИЙ БЕГА ★★★★
		if (g_vecAssassinWishDir.Length() > 0.1f)
		{
			return GetScheduleOfType(SCHED_CHASE_ENEMY);
		}

		return GetScheduleOfType(SCHED_ALERT_STAND);
	}









	// ОРИГИНАЛЬНАЯ ЛОГИКА AI

	switch	( m_MonsterState )
	{
	case MONSTERSTATE_IDLE:
	case MONSTERSTATE_ALERT:
		{
			if ( HasConditions ( bits_COND_HEAR_SOUND ))
			{
				CSound *pSound;
				pSound = PBestSound();

				ASSERT( pSound != NULL );
				if ( pSound && (pSound->m_iType & bits_SOUND_DANGER) )
				{
					return GetScheduleOfType( SCHED_TAKE_COVER_FROM_BEST_SOUND );
				}
				if ( pSound && (pSound->m_iType & bits_SOUND_COMBAT) )
				{
					return GetScheduleOfType( SCHED_INVESTIGATE_SOUND );
				}
			}
		}
		break;

	case MONSTERSTATE_COMBAT:
		{
// dead enemy
			if ( HasConditions( bits_COND_ENEMY_DEAD ) )
			{
				// call base class, all code to handle dead enemies is centralized there.
				return CBaseMonster :: GetSchedule();
			}

			// flying?
			if ( pev->movetype == MOVETYPE_TOSS)
			{
				if (pev->flags & FL_ONGROUND)
				{
					// ALERT( at_console, "landed\n");
					// just landed
					pev->movetype = MOVETYPE_STEP;
					return GetScheduleOfType ( SCHED_ASSASSIN_JUMP_LAND );
				}
				else
				{
					// ALERT( at_console, "jump\n");
					// jump or jump/shoot
					if ( m_MonsterState == MONSTERSTATE_COMBAT )
						return GetScheduleOfType ( SCHED_ASSASSIN_JUMP );
					else
						return GetScheduleOfType ( SCHED_ASSASSIN_JUMP_ATTACK );
				}
			}

			if ( HasConditions ( bits_COND_HEAR_SOUND ))
			{
				CSound *pSound;
				pSound = PBestSound();

				ASSERT( pSound != NULL );
				if ( pSound && (pSound->m_iType & bits_SOUND_DANGER) )
				{
					return GetScheduleOfType( SCHED_TAKE_COVER_FROM_BEST_SOUND );
				}
			}

			if ( HasConditions ( bits_COND_LIGHT_DAMAGE ) )
			{
				m_iFrustration++;
			}
			if ( HasConditions ( bits_COND_HEAVY_DAMAGE ) )
			{
				m_iFrustration++;
			}

		// jump player!
			if ( HasConditions ( bits_COND_CAN_MELEE_ATTACK1 ) )
			{
				// ALERT( at_console, "melee attack 1\n");
				return GetScheduleOfType ( SCHED_MELEE_ATTACK1 );
			}

		// throw grenade
			if ( HasConditions ( bits_COND_CAN_RANGE_ATTACK2 ) )
			{
				// ALERT( at_console, "range attack 2\n");
				return GetScheduleOfType ( SCHED_RANGE_ATTACK2 );
			}

		// spotted
			if ( HasConditions ( bits_COND_SEE_ENEMY ) && HasConditions ( bits_COND_ENEMY_FACING_ME ) )
			{
				// ALERT( at_console, "exposed\n");
				m_iFrustration++;
				return GetScheduleOfType ( SCHED_ASSASSIN_EXPOSED );
			}

		// can attack
			if ( HasConditions ( bits_COND_CAN_RANGE_ATTACK1 ) )
			{
				// ALERT( at_console, "range attack 1\n");
				m_iFrustration = 0;
				return GetScheduleOfType ( SCHED_RANGE_ATTACK1 );
			}

			if ( HasConditions ( bits_COND_SEE_ENEMY ) )
			{
				// ALERT( at_console, "face\n");
				return GetScheduleOfType ( SCHED_COMBAT_FACE );
			}

		// new enemy
			if ( HasConditions ( bits_COND_NEW_ENEMY ) )
			{
				// ALERT( at_console, "take cover\n");
				return GetScheduleOfType ( SCHED_TAKE_COVER_FROM_ENEMY );
			}

			// ALERT( at_console, "stand\n");
			return GetScheduleOfType ( SCHED_ALERT_STAND );
		}
		break;
	}

	return CBaseMonster :: GetSchedule();
}

//=========================================================
//=========================================================
Schedule_t* CHAssassin :: GetScheduleOfType ( int Type ) 
{
	// ALERT( at_console, "%d\n", m_iFrustration );
	switch	( Type )
	{
	case SCHED_TAKE_COVER_FROM_ENEMY:
		if (pev->health > 30)
			return slAssassinTakeCoverFromEnemy;
		else
			return slAssassinTakeCoverFromEnemy2;
	case SCHED_TAKE_COVER_FROM_BEST_SOUND:
		return slAssassinTakeCoverFromBestSound;
	case SCHED_ASSASSIN_EXPOSED:
		return slAssassinExposed;
	case SCHED_FAIL:
		if (m_MonsterState == MONSTERSTATE_COMBAT)
			return slAssassinFail;
		break;
	case SCHED_ALERT_STAND:
		if (m_MonsterState == MONSTERSTATE_COMBAT)
			return slAssassinHide;
		break;
	case SCHED_CHASE_ENEMY:
		return slAssassinHunt;
	case SCHED_MELEE_ATTACK1:
		if (pev->flags & FL_ONGROUND)
		{
			if (m_flNextJump > gpGlobals->time)
			{
				// can't jump yet, go ahead and fail
				return slAssassinFail;
			}
			else
			{
				return slAssassinJump;
			}
		}
		else
		{
			return slAssassinJumpAttack;
		}
	case SCHED_ASSASSIN_JUMP:
	case SCHED_ASSASSIN_JUMP_ATTACK:
		return slAssassinJumpAttack;
	case SCHED_ASSASSIN_JUMP_LAND:
		return slAssassinJumpLand;
	}

	return CBaseMonster :: GetScheduleOfType( Type );
}

#endif
