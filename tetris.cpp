#include <windows.h>

HINSTANCE hInstance;
HWND hMainWindow;

HDC hMemDC, hBlockDC;
HBITMAP hMemPrev, hBlockPrev;

typedef struct _TAG_POSITION {
    int x;
    int y;
} POSITION;

typedef struct _TAG_BLOCK {
    int rotate;
    POSITION p[3];
} BLOCK;

BLOCK block[8] = {
    {1, {{0,  0},{0, 0}, {0 ,0}}},  // null
    {2, {{0, -1},{0, 1}, {0 ,2}}},  // tetris
    {4, {{0, -1},{0, 1}, {1 ,1}}},  // L1
    {4, {{0, -1},{0, 1}, {-1,1}}},  // L2
    {2, {{0, -1},{1, 0}, {1 ,1}}},  // key1
    {2, {{0, -1},{-1,0}, {-1,1}}},  // key2
    {1, {{0,  1},{1, 0}, {1 ,1}}},  // square
    {4, {{0, -1},{1, 0}, {-1 ,0}}},  // T
};

typedef struct _TAG_STATUS {
    int x;
    int y;
    int type;
    int rotate;
} STATUS;

bool putBlock(STATUS s, bool action = false) {
    if(board[s.x][s.y] != 0) {
        return false;
    }


    if(action) {
        board[s.x][s.y] = s.type;
    }


    for(int i = 0; i < 3; i++) {
        int dx = block[s.type].p[i].x;
        int dy = block[s.type].p[i].y;
        int r = s.rotate % block[s.type].rotate;
        for(int j = 0; j < r; j++) {
            int nx = dx, ny = dy;
            dx = ny; dy = -nx;
        }
        if(board[s.x + dx][s.y + dy] != 0) {
            return false;
        }
        if(action) {
            board[s.x + dx][s.y + dy] = s.type;
        }
    }
    if(!action) {
        putBlock(s, true);
    }
    return true;
}

int board[12][25];

void showBoard() {
    for(int x = 1; x <= 10; x++) {
        for(int y = 1; y <= 20; y++) {
            BitBlt(hMemDC, (x - 1) * 24, (20 -y) * 24, 24, 24, hBlockDC, 0, board[x][y] * 24, SRCCOPY);
        }
    }
}


LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch(msg) {
        case WM_CREATE: {
            for(int x = 0; x < 12; x++) {
                for(int y = 0; y < 25; y++) {
                    if(x == 0 || x == 11 || y == 0) {
                        board[x][y] = 1;
                    } else {
                        board[x][y] = 0;
                    }
                }
            }
            
            // debug
            STATUS s;
            s.x = 5;
            s.y = 10;
            s.type = 5;
            s.rotate = 3;
            putBlock(s);
            HDC hdc = GetDC(hWnd);
            
            hMemDC = CreateCompatibleDC(hdc);
            HBITMAP hBitmap = CreateCompatibleBitmap(hdc, 24 * 10, 24 * 20);
            hMemPrev = (HBITMAP)SelectObject(hMemDC, hBitmap);
            
            hBlockDC = CreateCompatibleDC(hdc);
            hBitmap = LoadBitmap(hInstance, "BLOCKS");
            hBlockPrev = (HBITMAP)SelectObject(hBlockDC, hBitmap);
            
            // debug
            BitBlt(hMemDC, 0, 0, 24, 24, hBlockDC, 0, 0, SRCCOPY);
            
            ReleaseDC(hWnd, hdc);
            break;
        }
        case WM_PAINT: {
            showBoard();
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            BitBlt(hdc, 0, 0, 24 * 10, 24 * 20, hMemDC, 0, 0, SRCCOPY);
            EndPaint(hWnd, &ps);
            break;
        }
        case WM_DESTROY: {
            HBITMAP hBitmap = (HBITMAP)SelectObject(hMemDC, hMemPrev);
            DeleteObject(hBitmap);
            DeleteObject(hMemDC);
            
            hBitmap = (HBITMAP)SelectObject(hBlockDC, hBlockPrev);
            DeleteObject(hBitmap);
            DeleteObject(hBlockDC);
            
            PostQuitMessage(0);
            break;
        }
        default:
            return DefWindowProc(hWnd, msg, wParam, lParam);
    }
    return 0;
}

int WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR cmdLine, int cmdShow) {
    hInstance = hInst;
    WNDCLASSEX wc;
    static LPCTSTR pClassName = "NicoNicoProgramming2";  // クラス名

    wc.cbSize        = sizeof(WNDCLASSEX);               // 構造体サイズ
    wc.style         = CS_HREDRAW | CS_VREDRAW;          // クラススタイル
    wc.lpfnWndProc   = (WNDPROC)WndProc;
    wc.cbClsExtra    = 0;                                // 補足メモリブロック
    wc.cbWndExtra    = 0;                                // 　のサイズ
    wc.hInstance     = hInst;                            // インスタンス
    wc.hIcon         = NULL;                             // アイコン
    wc.hCursor       = LoadCursor(NULL,IDC_ARROW);       // カーソル
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);         // 背景色
    wc.lpszMenuName  = NULL;                             // メニュー
    wc.lpszClassName = pClassName;                       // クラス名
    wc.hIconSm       = NULL;                             // 小さいアイコン

    if (!RegisterClassEx(&wc)) return FALSE;             // 登録

    RECT r;
    r.left = r.top = 0;
    r.right = 24 * 10;
    r.bottom = 24 * 20;
    AdjustWindowRectEx(&r, WS_OVERLAPPED | WS_MINIMIZEBOX | WS_SYSMENU | WS_CAPTION, false, 0);

    hMainWindow = CreateWindow(pClassName, "Nico Nico Programming2", WS_OVERLAPPED | WS_MINIMIZEBOX | WS_SYSMENU | WS_CAPTION,
        CW_USEDEFAULT, CW_USEDEFAULT, r.right - r.left, r.bottom - r.top,
        NULL, NULL, hInst, NULL);
    
    ShowWindow(hMainWindow, SW_SHOW);
    
    MSG msg;
    while(GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}
