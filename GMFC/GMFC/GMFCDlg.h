
// GMFCDlg.h: 헤더 파일
//

#pragma once
#include <vector>
#include <thread>
#include <mutex>

#define WM_UPDATE_RANDOM_POS (WM_USER + 1001)

// CGMFCDlg 대화 상자
class CGMFCDlg : public CDialogEx
{
// 생성입니다.
public:
	CGMFCDlg(CWnd* pParent = nullptr);	// 표준 생성자입니다.

// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_GMFC_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 지원입니다.


// 구현입니다.
protected:
	HICON m_hIcon;

	// 생성된 메시지 맵 함수
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()

private:
	std::vector<CPoint> m_points;       // 점 3개
	bool m_isDragging;					// 드래그 여부
	int  m_dragIndex;					// 드래그하는 점
	int  m_smallRadius;                 // UI에서 입력
	int  m_thickness;                   // UI에서 입력

	CImage m_image;

	std::mutex m_mutex;					// 쓰레드 충돌 방지용 뮤텍스

public:
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);

	///////////
	bool PointNear(const CPoint& p, const CPoint& q, int range);
	void DrawRawCircle(unsigned char* fm, int pitch, int width, int height,
		int centerX, int centerY, int radius, int thickness, bool bFill);

	// 세 개의 점 A, B, C를 모두 지나는 외접원의 중심과 반지름을 계산하는 함수
	bool CalcCircumcircle(const CPoint& A, const CPoint& B, const CPoint& C, CPoint& center, double& radius);

	// 쓰레드로부터 갱신 요청을 받을 메시지 핸들러
	afx_msg LRESULT OnUpdateRandomPos(WPARAM wParam, LPARAM lParam);

	
	afx_msg void OnBnClickedBtnReset();
	afx_msg void OnBnClickedBtnRandom();
};
