/*
 * menu.c
 * Location: Motion/Src/menu.c
 */

#include "menu.h"
#include "lcd.h"
#include "switch.h"

#include <stddef.h> // for NULL

// --- [1] 메뉴 시스템 자료구조 정의 ---

typedef struct Menu_t Menu_t; // Forward Declaration

typedef struct {
	const char *name;           // 화면에 표시될 이름
	void (*func)(void);         // 실행할 함수 (Leaf 노드일 경우)
	const Menu_t *childMenu;    // 하위 메뉴 포인터 (없으면 NULL)
} MenuItem_t;

struct Menu_t {
	const char *title;          // 메뉴 제목
	const MenuItem_t *items;    // 메뉴 항목 배열
	uint8_t itemCount;          // 항목 개수
	const Menu_t *parentMenu;   // 상위 메뉴 포인터 (뒤로가기용)
};

// --- [2] 테스트 함수 프로토타입 (실제 구현은 파일 하단) ---

// Top Level
static void Run_SearchMode(void);
static void Run_FastMode(void);

// Sensor Tests
static void Test_ToF_Sensor(void);
static void Test_Gyro_Sensor(void);

// Motor Tests (Submenu Items)
static void Test_DRV8316_Comm(void);
static void Test_Encoder_Val(void);
static void Test_FOC_Control(void);
static void Test_PI_Gain_Step(void);

// --- [3] 메뉴 계층 구조 정의 (하위 -> 상위 순서로 정의해야 함) ---

/* === 3. Motor Test Sub Menu === */
static const MenuItem_t items_MotorTest[] = { { "DRV8316 Check",
		Test_DRV8316_Comm, NULL }, { "Encoder View", Test_Encoder_Val, NULL }, {
		"FOC Spin Test", Test_FOC_Control, NULL }, { "PI Step Resp.",
		Test_PI_Gain_Step, NULL }, { "<- Back", NULL, NULL } // 뒤로가기 (로직에서 처리)
};

static const Menu_t menu_MotorTest = { .title = "[ Motor Test ]", .items =
		items_MotorTest, .itemCount = sizeof(items_MotorTest)
		/ sizeof(MenuItem_t), .parentMenu = NULL // 런타임에 할당 or 전역에서 연결
		};

/* === 2. Test Menu (Mid Level) === */
static const MenuItem_t items_TestMenu[] = { { "ToF Sensors", Test_ToF_Sensor,
NULL }, { "Gyroscope", Test_Gyro_Sensor, NULL }, { "Motor / FOC >",
NULL, &menu_MotorTest }, // 하위 메뉴 연결
		{ "<- Back", NULL, NULL } };

static const Menu_t menu_TestRoot = { .title = "[ H/W Test ]", .items =
		items_TestMenu,
		.itemCount = sizeof(items_TestMenu) / sizeof(MenuItem_t), .parentMenu =
		NULL };

/* === 1. Main Root Menu === */
static const MenuItem_t items_Main[] = { { "Search Run", Run_SearchMode, NULL },
		{ "Fast Run", Run_FastMode, NULL }, { "Test Mode >", NULL,
				&menu_TestRoot },  // 하위 메뉴 연결
		};

static const Menu_t menu_Main =
		{ .title = "[ Main Menu ]", .items = items_Main, .itemCount =
				sizeof(items_Main) / sizeof(MenuItem_t), .parentMenu = NULL };

// --- [4] 메뉴 상태 관리 변수 ---
static const Menu_t *pCurrentMenu;
static uint8_t cursorIndex = 0;
static bool updateScreen = true;

// --- [5] 내부 유틸리티 함수 ---

static void Menu_SetParentLinks(void) {
	// 구조체 초기화 시 const로 인해 parent 설정이 어려우므로, 초기화 함수에서 연결
	// (C언어의 한계로 수동 연결이 가장 확실함)
	((Menu_t*) &menu_TestRoot)->parentMenu = &menu_Main;
	((Menu_t*) &menu_MotorTest)->parentMenu = &menu_TestRoot;
}

static void Menu_Render(void) {
	if (!updateScreen)
		return;

	LCD_Clear();
	// 제목 출력 (노란색)
	Set_Color(YELLOW, BLACK);
	LCD_Printf(0, 0, (char*) pCurrentMenu->title);

	// 항목 출력
	for (int i = 0; i < pCurrentMenu->itemCount; i++) {
		Set_Color(WHITE, BLACK);

		if (i == cursorIndex) {
			Set_Color(BLACK, GREEN);
		}
		LCD_Printf(0, i + 1, (char*) pCurrentMenu->title);
	}
	updateScreen = false;
}

// --- [6] Public Functions ---

void Menu_Init(void) {
	LCD_Clear();

	Menu_SetParentLinks(); // 부모-자식 관계 연결

	pCurrentMenu = &menu_Main;
	cursorIndex = 0;
	updateScreen = true;
}

void Menu_Loop(void) {
	uint8_t swEvent = Switch_Read();

	// 1. 커서 이동 (SW1)
	if (swEvent & SW_EVENT_1) {
		cursorIndex++;
		if (cursorIndex >= pCurrentMenu->itemCount) {
			cursorIndex = 0;
		}
		updateScreen = true;
	}

	// 2. 선택 / 실행 (SW2)
	if (swEvent & SW_EVENT_2) {
		const MenuItem_t *pItem = &pCurrentMenu->items[cursorIndex];

		// Case A: 뒤로가기 ("<- Back" 항목 선택 시)
		// 로직: 함수도 없고 자식도 없는데 부모가 있으면 Back으로 간주 (혹은 이름으로 체크)
		if (pItem->func == NULL && pItem->childMenu == NULL
				&& pCurrentMenu->parentMenu != NULL) {
			pCurrentMenu = pCurrentMenu->parentMenu;
			cursorIndex = 0;
			updateScreen = true;
			return; // 루프 종료
		}

		// Case B: 하위 메뉴 진입
		if (pItem->childMenu != NULL) {
			pCurrentMenu = pItem->childMenu;
			cursorIndex = 0;
			updateScreen = true;
		}
		// Case C: 기능(함수) 실행
		else if (pItem->func != NULL) {
			LCD_Clear();
			pItem->func(); // Blocking 실행

			// 실행 후 복귀
			updateScreen = true;
			// 키 입력 버퍼 비우기 등 필요 시 추가
		}
	}

	Menu_Render();
}

// --- [7] 기능 구현부 (상세 테스트 코드) ---

/* 공통 유틸리티: SW2 누르면 탈출 */
static bool Check_Exit(void) {
	if (Switch_Read() & SW_EVENT_BOTH)
		return true;
	return false;
}

static void Run_SearchMode(void) {
	LCD_Clear();
	LCD_Printf(0, 0, "Search Running");
	HAL_Delay(1000);
}

static void Run_FastMode(void) {
	LCD_Clear();
	LCD_Printf(0, 0, "Fast Running");
	HAL_Delay(1000);
}

static void Test_ToF_Sensor(void) {
	LCD_Clear();
	LCD_Printf(0, 0, "ToF Test");
	while (!Check_Exit()) {
	}
}

static void Test_Gyro_Sensor(void) {
	LCD_Clear();
	LCD_Printf(0, 0, "Gyro Test");
	while (!Check_Exit()) {

	}
}

// === Motor Test Implementation ===

static void Test_DRV8316_Comm(void) {
	LCD_Clear();
	LCD_Printf(0, 0, "DRV8316C Test");
	while (!Check_Exit()) {
	}
}

static void Test_Encoder_Val(void) {
	LCD_Clear();
	LCD_Printf(0, 0, "Encoder Test");
	while (!Check_Exit()) {
	}
}

static void Test_FOC_Control(void) {
	LCD_Clear();
	LCD_Printf(0, 0, "FOC Test");
	while (!Check_Exit()) {
	}
}

static void Test_PI_Gain_Step(void) {
	LCD_Clear();
	LCD_Printf(0, 0, "PI Gain Test");
	while (!Check_Exit()) {
	}
}
