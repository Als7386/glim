
// GMFCDlg.cpp: 구현 파일
//

#include "pch.h"
#include "framework.h"
#include "GMFC.h"
#include "GMFCDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// 응용 프로그램 정보에 사용되는 CAboutDlg 대화 상자입니다.

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

// 구현입니다.
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CGMFCDlg 대화 상자



CGMFCDlg::CGMFCDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_GMFC_DIALOG, pParent),
	m_isDragging(false),
	m_dragIndex(-1),
	m_smallRadius(10),
	m_thickness(3)
	
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CGMFCDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CGMFCDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_BN_CLICKED(IDC_BTN_RESET, &CGMFCDlg::OnBnClickedBtnReset)
	ON_BN_CLICKED(IC_BTN_RANDOM, &CGMFCDlg::OnBnClickedBtnRandom)
	ON_MESSAGE(WM_UPDATE_RANDOM_POS, &CGMFCDlg::OnUpdateRandomPos)
END_MESSAGE_MAP()


// CGMFCDlg 메시지 처리기

BOOL CGMFCDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 시스템 메뉴에 "정보..." 메뉴 항목을 추가합니다.

	// IDM_ABOUTBOX는 시스템 명령 범위에 있어야 합니다.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != nullptr)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 이 대화 상자의 아이콘을 설정합니다.  응용 프로그램의 주 창이 대화 상자가 아닐 경우에는
	//  프레임워크가 이 작업을 자동으로 수행합니다.
	SetIcon(m_hIcon, TRUE);			// 큰 아이콘을 설정합니다.
	SetIcon(m_hIcon, FALSE);		// 작은 아이콘을 설정합니다.

	// TODO: 여기에 추가 초기화 작업을 추가합니다.

	ModifyStyle(0, WS_CLIPCHILDREN);// 버튼 깜빡임 방지
	// 초기값 설정 반지름, 두께
	SetDlgItemInt(IDC_EDIT_RADIUS, m_smallRadius);
	SetDlgItemInt(IDC_EDIT_THICKNESS, m_thickness);



	return TRUE;  // 포커스를 컨트롤에 설정하지 않으면 TRUE를 반환합니다.
}

void CGMFCDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// 대화 상자에 최소화 단추를 추가할 경우 아이콘을 그리려면
//  아래 코드가 필요합니다.  문서/뷰 모델을 사용하는 MFC 애플리케이션의 경우에는
//  프레임워크에서 이 작업을 자동으로 수행합니다.

void CGMFCDlg::OnPaint()
{
	CPaintDC dc(this);
	CRect rect;
	GetClientRect(&rect);

	int w = rect.Width();
	int h = rect.Height();

	// CImage 버퍼 생성
	if (m_image.IsNull() || m_image.GetWidth() != w || m_image.GetHeight() != h)
	{
		if (!m_image.IsNull()) m_image.Destroy();
		m_image.Create(w, h, 24);
	}
	// 이미지 포인터와 피치 가져오기
	unsigned char* fm = (unsigned char*)m_image.GetBits();
	int pitch = m_image.GetPitch();
	// 반지름, 두께 입력값을 적용
	m_smallRadius = GetDlgItemInt(IDC_EDIT_RADIUS);
	m_thickness = GetDlgItemInt(IDC_EDIT_THICKNESS);


	// 가장 안전한 배경 초기화 (흰색)
	unsigned char* pRow = fm;
	for (int y = 0; y < h; y++) {
		memset(pRow, 255, w * 3); // 한 줄을 흰색(0xFF)으로
		pRow += pitch;
	}

	//뮤텍스 잠금
	std::lock_guard<std::mutex> lock(m_mutex);

	// 점 그리기
	std::vector<CPoint> localPoints = m_points;
	for (const auto& pt : localPoints)
	{
		// 10: 반지름, true: 채우기
		DrawRawCircle(fm, pitch, w, h, pt.x, pt.y, m_smallRadius, 0, true);
	}

	// 외접원 그리기
	if (localPoints.size() >= 3)
	{
		CPoint center;
		double radiusD = 0.0;
		if (CalcCircumcircle(localPoints[0], localPoints[1], localPoints[2], center, radiusD))
		{
			// false: 채우기 없음(테두리만)
			DrawRawCircle(fm, pitch, w, h, center.x, center.y, (int)radiusD, m_thickness, false);
		}
	}

	//완성된 이미지 화면에 복사 
	m_image.Draw(dc.m_hDC, 0, 0);

}

// 사용자가 최소화된 창을 끄는 동안에 커서가 표시되도록 시스템에서
//  이 함수를 호출합니다.
HCURSOR CGMFCDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


void CGMFCDlg::OnLButtonDown(UINT nFlags, CPoint point)
{
	bool haveThree = (m_points.size() >= 3);

	if (haveThree)
	{
		for (int i = 0; i < 3 && i < m_points.size(); ++i)
		{
			if (PointNear(point, m_points[i], max(10, m_smallRadius)))
			{
				m_isDragging = true;
				m_dragIndex = (int)i;
				return;
			}
		}
	}

	if (m_points.size() < 3)
	{
		m_points.push_back(point);
		if (m_points.size() >= 1)
		{
			CString s;
			s.Format(L"(%d, %d)", m_points[0].x, m_points[0].y);
			SetDlgItemText(IDC_EDIT_P1, s);
		}
		if (m_points.size() >= 2)
		{
			CString s;
			s.Format(L"(%d, %d)", m_points[1].x, m_points[1].y);
			SetDlgItemText(IDC_EDIT_P2, s);
		}
		if (m_points.size() >= 3)
		{
			CString s;
			s.Format(L"(%d, %d)", m_points[2].x, m_points[2].y);
			SetDlgItemText(IDC_EDIT_P3, s);
		}
	}
	Invalidate(FALSE);
}

void CGMFCDlg::OnLButtonUp(UINT nFlags, CPoint point)
{
	if (m_isDragging)
	{
		m_isDragging = false;
		m_dragIndex = -1;
		Invalidate(FALSE);
	}
}

void CGMFCDlg::OnMouseMove(UINT nFlags, CPoint point)
{
	if (m_isDragging && m_dragIndex >= 0)
	{
		if (m_dragIndex < (int)m_points.size())
		{
			m_points[m_dragIndex] = point;

			CString s;
			s.Format(L"(%d, %d)", m_points[m_dragIndex].x, m_points[m_dragIndex].y);
			if (m_dragIndex == 0) SetDlgItemText(IDC_EDIT_P1, s);
			else if (m_dragIndex == 1) SetDlgItemText(IDC_EDIT_P2, s);
			else if (m_dragIndex == 2) SetDlgItemText(IDC_EDIT_P3, s);
		}

		Invalidate(FALSE);
	}
}

bool CGMFCDlg::PointNear(const CPoint& p, const CPoint& q, int range)
{
	// p마우스 클릭 좌표, q 화면에 그려진 점의 좌표, range 허용범위
	// 마우스로 클릭한 위치가 점 위인지 검사 하는 함수
	int dx = p.x - q.x;
	int dy = p.y - q.y;
	return (dx * dx + dy * dy) <= (range * range);
}
inline void DrawPixel(unsigned char* fm, int pitch, int width, int height, int x, int y)
{
	// 화면 밖으로 나가는 것 방지 (유효성 검사)
	if (x < 0 || x >= width || y < 0 || y >= height) return;

	// 메모리 주소 계산: y번째 줄의 시작 주소 + x번째 픽셀의 위치
	unsigned char* pPixel = fm + (y * pitch) + (x * 3);
	for (int i = 0; i < 3; i++) {	// 검은색 초기화
		pPixel[i] = 0;
	}
}
void CGMFCDlg::DrawRawCircle(unsigned char* fm, int pitch, int width, int height, int centerX, int centerY, int radius, int thickness, bool bFill)
{
	// 검사할 영역(Bounding Box) 설정 - 전체를 다 돌면 느리므로 원 주변만 검사
	int left = max(0, centerX - radius - thickness);
	int right = min(width - 1, centerX + radius + thickness);
	int top = max(0, centerY - radius - thickness);
	int bottom = min(height - 1, centerY + radius + thickness);

	// 원의 방정식: (x-cx)^2 + (y-cy)^2 <= r^2
	// 성능을 위해 제곱값 미리 계산
	int rSq = radius * radius;
	// 두께가 있는 테두리 원의 경우, 안쪽 원의 반지름 제곱 (r - thickness)^2
	int innerRSq = (radius - thickness) * (radius - thickness);
	if (innerRSq < 0) innerRSq = 0;

	for (int y = top; y <= bottom; ++y)
	{
		for (int x = left; x <= right; ++x)
		{
			int dx = x - centerX;
			int dy = y - centerY;
			int distSq = dx * dx + dy * dy;

			bool bDraw = false;

			if (bFill)
			{
				// 채워진 원: 중심 거리가 반지름보다 작으면 그림
				if (distSq <= rSq) bDraw = true;
			}
			else
			{
				// 테두리 원: 바깥 반지름보다 작고, 안쪽 반지름보다 크면 그림
				if (distSq <= rSq && distSq >= innerRSq) bDraw = true;
			}

			if (bDraw)
			{
				DrawPixel(fm, pitch, width, height, x, y);
			}
		}
	}
}

bool CGMFCDlg::CalcCircumcircle(const CPoint& A, const CPoint& B, const CPoint& C, CPoint& center, double& radius)
{
	double x1 = A.x, y1 = A.y;
	double x2 = B.x, y2 = B.y;
	double x3 = C.x, y3 = C.y;

	double d = 2.0 * (x1 * (y2 - y3) + x2 * (y3 - y1) + x3 * (y1 - y2));
	if (fabs(d) < 1e-7) return false;

	double x1s = x1 * x1 + y1 * y1;
	double x2s = x2 * x2 + y2 * y2;
	double x3s = x3 * x3 + y3 * y3;

	double ux = (x1s * (y2 - y3) + x2s * (y3 - y1) + x3s * (y1 - y2)) / d;
	double uy = (x1s * (x3 - x2) + x2s * (x1 - x3) + x3s * (x2 - x1)) / d;

	center.x = (int)round(ux);
	center.y = (int)round(uy);

	radius = hypot(ux - x1, uy - y1);
	return true;
}


void CGMFCDlg::OnBnClickedBtnReset()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.

	m_points.clear();
	SetDlgItemText(IDC_EDIT_P1, L"");
	SetDlgItemText(IDC_EDIT_P2, L"");
	SetDlgItemText(IDC_EDIT_P3, L"");

	Invalidate(FALSE);

}

void CGMFCDlg::OnBnClickedBtnRandom()
{
	if (m_points.size() < 3) {
		return;
	}

	// 화면 크기 가져오기 쓰레드 내부에서 GetClientRect 사용 불가하므로 미리 계산
	CRect rect;
	GetClientRect(&rect);
	int width = rect.Width();
	int height = rect.Height();

	// 별도 쓰레드 생성 람다 함수 사용
	// this 포인터를 캡처하여 멤버 변수에 접근 가능하게 함
	std::thread randomThread([this, width, height]() {

		// 총 10회 반복
		for (int i = 0; i < 10; ++i)
		{
			// 데이터 변경 영역 Critical Section이므로 lock
			{
				std::lock_guard<std::mutex> lock(m_mutex);

				// 3개의 점을 랜덤 위치로 이동
				for (auto& pt : m_points) {
					pt.x = rand() % width;
					pt.y = rand() % height;
				}
			}

			// 메인 윈도우에 PostMessage
			// m_hWnd가 유효한지 확인
			if (::IsWindow(m_hWnd)) {
				PostMessage(WM_UPDATE_RANDOM_POS, 0, 0);
			}

			// 0.5초 대기 (500ms) - 초당 2회
			std::this_thread::sleep_for(std::chrono::milliseconds(500));
		}
		});

	// 쓰레드를 메인 프로세스와 분리하여 백그라운드에서 실행되게 함
	randomThread.detach();
}
LRESULT CGMFCDlg::OnUpdateRandomPos(WPARAM wParam, LPARAM lParam)
{
	// 좌표 텍스트 갱신 (P1, P2, P3 Edit Control)

	std::lock_guard<std::mutex> lock(m_mutex);

	if (m_points.size() >= 1) {
		CString s; s.Format(L"(%d, %d)", m_points[0].x, m_points[0].y);
		SetDlgItemText(IDC_EDIT_P1, s);
	}
	if (m_points.size() >= 2) {
		CString s; s.Format(L"(%d, %d)", m_points[1].x, m_points[1].y);
		SetDlgItemText(IDC_EDIT_P2, s);
	}
	if (m_points.size() >= 3) {
		CString s; s.Format(L"(%d, %d)", m_points[2].x, m_points[2].y);
		SetDlgItemText(IDC_EDIT_P3, s);
	}

	// 화면 갱신 요청 (OnPaint 호출됨)
	Invalidate(FALSE);
	return 0;
}
