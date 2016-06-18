// CPointCloudView.cpp : implementation of the CPointCloudView class
//

#include "stdafx.h"
#include "PointCloud.h"

#include "PointCloudDoc.h"
#include "PointCloudView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define PROCESS_ID_TRANSLATION	1    //平移
#define PROCESS_ID_SCALE		2	 //缩放
#define PROCESS_ID_ROTATION		3	 //旋转
/////////////////////////////////////////////////////////////////////////////
// CPointCloudView

IMPLEMENT_DYNCREATE(CPointCloudView, CView)

BEGIN_MESSAGE_MAP(CPointCloudView, CView)
	//{{AFX_MSG_MAP(CPointCloudView)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_DESTROY()
	ON_WM_ERASEBKGND()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MBUTTONDOWN()
	ON_WM_MBUTTONUP()
	ON_WM_MOUSEWHEEL()
	ON_WM_RBUTTONDOWN()
	ON_WM_RBUTTONUP()
	//}}AFX_MSG_MAP
	// Standard printing commands
	ON_COMMAND(ID_FILE_PRINT, CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, CView::OnFilePrintPreview)
//	ON_WM_KEYDOWN()
//	ON_WM_KEYUP()
//	ON_WM_PAINT()
ON_WM_SYSKEYDOWN()
ON_WM_SYSKEYUP()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPointCloudView construction/destruction

CPointCloudView::CPointCloudView()
{
	m_pDC=NULL;
	m_bStatusbar=FALSE;
	m_bToolbar=FALSE;

	m_bShowPointCloud=FALSE;
	alt = false;
	glTransX=glTransY=glTransZ=0;			//当前平移参数
	glCurTransX=glCurTransY=glCurTransZ=0;
	glRotX=glRotY=glRotZ=0.0;				//当前旋转参数
	glScaleX=glScaleY=glScaleZ=1.0;			//当前缩放系数
	glCurRotX=glCurRotY=glCurRotZ=0.0;
    m_bScale=FALSE;
	m_bRotation=FALSE;
	m_bMoving=FALSE;
	bInitOpenGL=FALSE;
	

}

CPointCloudView::~CPointCloudView()
{
}

BOOL CPointCloudView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs
	cs.style=WS_CHILD|WS_VISIBLE|WS_CLIPCHILDREN|WS_CLIPSIBLINGS;
	return CView::PreCreateWindow(cs);
}

/////////////////////////////////////////////////////////////////////////////
// CPointCloudView drawing

void CPointCloudView::OnDraw(CDC* pDC)
{
	CPointCloudDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	// TODO: add draw code for native data here

	/*static BOOL bBusy=FALSE;
	if (bBusy)
	{
		return;
	}*/
	//bBusy=TRUE;


	InitOpenGL();	
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	//glTranslatef(pDoc->m_ptBoxCenter.x,pDoc->m_ptBoxCenter.y,pDoc->m_ptBoxCenter.z);
    //	glTranslatef(0-glTransX,0-glTransY,0-glTransZ);	
	glTranslatef(0 + glTransX - glCurTransX*0.2, 0 + glTransY + glCurTransY*0.2, 0 + glTransZ - glCurTransZ*0.2);
	glRotatef(glCurRotX+glRotX,0.0, 1.0, 0.0);	//绕y轴旋转
	glRotatef(glCurRotY+glRotY,1.0, 0.0, 0.0);	//绕x轴旋转
	glRotatef(glCurRotZ+glRotZ,0.0, 0.0, 1.0);	//绕x轴旋转
	glScalef(glScaleX,glScaleY,glScaleZ);	
	glTranslatef(-pDoc->m_ptBoxCenter.x,-pDoc->m_ptBoxCenter.y,-pDoc->m_ptBoxCenter.z);
	glPushMatrix();//自定义
	glTranslatef(-pDoc->m_ptBoxCenter.x, -pDoc->m_ptBoxCenter.y, -pDoc->m_ptBoxCenter.z);

 //	glMatrixMode(GL_MODELVIEW);
	//glGetFloatv(GL_MODELVIEW_MATRIX,ModelViewMatrix);
// 	glTranslatef(0+glTransX,0+glTransY,0+glTransZ);	
	//绘图函数
	mydraw();
	// 完成绘制
	glFinish();
	glPopMatrix();
	glPopMatrix();

	SwapBuffers(wglGetCurrentDC());
//	glMatrixMode(GL_MODELVIEW);
//	glLoadMatrixf(ModelViewMatrix);
//	glMatrixMode(GL_PROJECTION);
//	glLoadMatrixf(ProMatrix);
	//bBusy=FALSE;
}

/////////////////////////////////////////////////////////////////////////////
// CPointCloudView printing

BOOL CPointCloudView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// default preparation
	return DoPreparePrinting(pInfo);
}

void CPointCloudView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add extra initialization before printing
}

void CPointCloudView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add cleanup after printing
}

/////////////////////////////////////////////////////////////////////////////
// CPointCloudView diagnostics

#ifdef _DEBUG
void CPointCloudView::AssertValid() const
{
	CView::AssertValid();
}

void CPointCloudView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CPointCloudDoc* CPointCloudView::GetDocument() // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CPointCloudDoc)));
	return (CPointCloudDoc*)m_pDocument;
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CPointCloudView message handlers



Point3D CPointCloudView::Get_3D_pos(CPoint Tpoint)
{
	CPointCloudDoc*pDoc = GetDocument();

	Point3D Rp;
	GLint viewport[4];
	GLdouble modelview[16];
	GLdouble projection[16];
	GLfloat winX, winY, winZ;
	GLdouble posX, posY, posZ;

	
	winX = (float)Tpoint.x;
	winY = viewport[3] - (float)Tpoint.y;

	glReadPixels((int)winX, (int)winY, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &winZ);//获取鼠标屏幕坐标处像素的深度Z
	gluUnProject(winX, winY, winZ, modelview, projection, viewport, &posX, &posY, &posZ);

	Rp.x = posX;
	Rp.y = posY;
	Rp.z = posZ;

	return Rp;

}
void CPointCloudView::mydraw()
{

	CPointCloudDoc*pDoc=GetDocument();
	ASSERT_VALID(pDoc);
	
	if (pDoc->m_ptVertexList.empty())
		return;


	glColor3f(0.0 , 255.0  , 1.0 );//	vector<POINT3D>::iterator pItPoint3d;
	vector<Point3D>::iterator pItPoint3d;
	glBegin(GL_POINTS);
	for (pItPoint3d = pDoc->m_ptVertexList.begin() ; 
	pItPoint3d != pDoc->m_ptVertexList.end() ; ++pItPoint3d)
		if (pItPoint3d->w > 0)
			glVertex3f(pItPoint3d->x , pItPoint3d->y ,pItPoint3d->z);
    glEnd();

	glBegin(GL_LINES);
	   // glTranslatef(12000000, 1200, -200);
	
		glColor3f(0.0,0.0,1.0);
		glVertex3f(pDoc->m_ptBoxCenter.x*2,pDoc->m_ptBoxCenter.y*2,pDoc->m_ptBoxCenter.z*2);
		glVertex3f(pDoc->m_dbDistance,0.0, 0.0);
		glColor3f(0.0,1.0,0.0);
		glVertex3f(pDoc->m_ptBoxCenter.x * 2, pDoc->m_ptBoxCenter.y * 2, pDoc->m_ptBoxCenter.z * 2);
		glVertex3f(0.0,pDoc->m_dbDistance, 0.0);
		glColor3f(1.0, 0.0, 0.0);
		glVertex3f(pDoc->m_ptBoxCenter.x * 2, pDoc->m_ptBoxCenter.y * 2, pDoc->m_ptBoxCenter.z * 2);
		glVertex3f(0.0, 0.0, pDoc->m_dbDistance);
	
	glEnd();
	

}

BOOL CPointCloudView::PiexlFormat()
{

	static PIXELFORMATDESCRIPTOR pfd=
	{
		sizeof(PIXELFORMATDESCRIPTOR),
			1,
			PFD_DRAW_TO_WINDOW|PFD_SUPPORT_OPENGL|PFD_DOUBLEBUFFER,
			PFD_TYPE_RGBA,
			24,
			0,0,0,0,0,0,
			0,
			0,
			0,
			0,0,0,0,
			32,
			0,
			0,
			PFD_MAIN_PLANE,
			0,
			0,0,0
	};
	int nPixelFormat;
	//设置像素格式

	if ( (nPixelFormat =ChoosePixelFormat(m_pDC->GetSafeHdc(),&pfd)) == 0 )
	{
		MessageBox("ChoosePixelFormat failed");
		return FALSE;
	}
	if (SetPixelFormat(m_pDC->GetSafeHdc(),nPixelFormat,&pfd) == FALSE)
	{
		MessageBox("SetPixelFormat failed");
		return FALSE;
	}
	return TRUE;
}

//初始化 
//测试像素格式，创建渲染描述表
void CPointCloudView::InitRC()
{
	//设置像素格式
	PIXELFORMATDESCRIPTOR pfd;
	int n;
	m_pDC=new CClientDC(this);
	ASSERT(m_pDC!=NULL);
	if (!PiexlFormat())
	{
		return;
	}
	//测试像素格式
	n=::GetPixelFormat(m_pDC->GetSafeHdc());
	::DescribePixelFormat(m_pDC->GetSafeHdc(),n,sizeof(pfd),&pfd);

	
	
	
	GetClientRect(&m_rectOld);
	m_hRC=wglCreateContext(m_pDC->m_hDC);

	//视点、视口设置
	wglMakeCurrent(m_pDC->m_hDC,m_hRC);
	glClearDepth(1.0f);
	glEnable(GL_DEPTH_TEST);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
    
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();


}

int CPointCloudView::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CView::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	// TODO: Add your specialized creation code here
	InitRC();		//完成渲染描述表使用
	//this->SetMenu(NULL);
	return 0;
}



void CPointCloudView::OnSize(UINT nType, int cx, int cy) 
{
	CView::OnSize(nType, cx, cy);
	
	// TODO: Add your message handler code here
	
	if (cy >0)
	{
		m_rectOld.right=cx;
		m_rectOld.bottom=cy;
		glMatrixMode(GL_PROJECTION);	
		glLoadIdentity();
		glViewport(0,0,cx,cy);
	
	}
	
	RedrawWindow();//显示更新
	
}

void CPointCloudView::OnDestroy() 
{
	CView::OnDestroy();
	
	// TODO: Add your message handler code here

	wglDeleteContext(m_hRC);
	if (m_pDC)							//删除与渲染描述表绑定的设备
	{
		delete m_pDC;
	}
	
}

BOOL CPointCloudView::OnEraseBkgnd(CDC* pDC) 
{
	// TODO: Add your message handler code here and/or call default
	
	return CView::OnEraseBkgnd(pDC);
}

void CPointCloudView::OnMouseMove(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default
	CPointCloudDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	// if(MK_MBUTTON*&&!MK_LBUTTON&&!MK_RBUTTON&&!MK_CONTROL&&!MK_SHIFT*/)为什么不行？？？

	//鼠标中键           按住拖动            平移
	if (m_bMoving)
	{
			glCurTransX = (GLfloat)(glDownX - point.x) / m_rectOld.Width() * 2 * pDoc->m_dbDistance;
			glCurTransY = (GLfloat)(glDownY - point.y) / m_rectOld.Height() * 2 * pDoc->m_dbDistance;
			Invalidate(FALSE);//显示更新	
	}

	if (m_bRotation)
	{
		glCurRotX=(double)(glDownX-point.x)*360/(double)m_rectOld.Width();
		glCurRotY=(double)(glDownY-point.y)*360/(double)m_rectOld.Height();
		Invalidate(FALSE);//显示更新
	}
	
	CView::OnMouseMove(nFlags, point);
}


void CPointCloudView::OnLButtonDown(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default
	//旋转
	if (alt==true)
	m_bMoving = !m_bMoving;
	if (m_bMoving)
	{

		glDownX = point.x;
		glDownY = point.y;
		glDownZ = 0;
		//alt = false;
	}
	CView::OnLButtonDown(nFlags, point);
}

void CPointCloudView::OnLButtonUp(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default

	CPointCloudDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (m_bMoving)
	{
		glCurTransX = (GLfloat)(glDownX - point.x) / m_rectOld.Width() * 2 * pDoc->m_dbDistance;
		glCurTransY = (GLfloat)(glDownY - point.y) / m_rectOld.Height() * 2 * pDoc->m_dbDistance;
		glTransX -= glCurTransX*0.2;
		glTransY += glCurTransY*0.2;
		glTransZ += glCurTransZ*0.2;
		// 		glMatrixMode(GL_MODELVIEW);
		// 		glTranslatef(glTransX,glTransY,glTransZ);
		//		glPushMatrix();//入栈
		glCurTransX = 0;
		glCurTransY = 0;
		glCurTransZ = 0;
		m_bMoving = FALSE;
		Invalidate(FALSE);
	}


	CView::OnLButtonUp(nFlags, point);
}

void CPointCloudView::OnMButtonDown(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default
	m_bRotation = TRUE;
	glDownX = point.x;
	glDownY = point.y;
	glDownZ = 0;
	//鼠标中键  按住拖动   平移




	CView::OnMButtonDown(nFlags, point);
}

void CPointCloudView::OnMButtonUp(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default


	CPointCloudDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	m_bRotation = FALSE;

	glCurRotX = double(glDownX - point.x)*360.0 / (double)m_rectOld.Width();
	glCurRotY = double(glDownY - point.y)*360.0 / (double)m_rectOld.Height();
	//转化为角度
	//glMatrixMode(GL_PROJECTION);
	//glLoadIdentity();
	//   	glTranslatef(pDoc->m_ptBoxCenter.x,pDoc->m_ptBoxCenter.y,pDoc->m_ptBoxCenter.z);
	//   	glRotatef(glCurRotX,0.0, 1.0, 0.0);	//绕y轴旋转
	//   	glRotatef(glCurRotY,1.0, 0.0, 0.0);	//绕x轴旋转
	//   	glTranslatef(-pDoc->m_ptBoxCenter.x,-pDoc->m_ptBoxCenter.y,-pDoc->m_ptBoxCenter.z);
	// 	glGetFloatv(GL_PROJECTION,ProMatrix);
	glRotX += glCurRotX;
	glRotY += glCurRotY;
	glRotZ += glCurRotZ;
	glCurRotX = 0;
	glCurRotY = 0;


	Invalidate(FALSE);
	CView::OnMButtonUp(nFlags, point);
}

BOOL CPointCloudView::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt) 
{
	// TODO: Add your message handler code here and/or call default
	if (glScaleX<10)//防止丢失视野
	glScaleX=glScaleY=glScaleZ+=(GLfloat)zDelta/(GLfloat)m_rectOld.Height();
	else
		glScaleX = glScaleY += (GLfloat)zDelta / (GLfloat)m_rectOld.Height();
	Invalidate(FALSE);
	return CView::OnMouseWheel(nFlags, zDelta, pt);
}

void CPointCloudView::OnRButtonDown(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default
// 	m_bRotation=TRUE;
// 	glDownX=point.x;
// 	glDownY=point.y;
// 	glDownZ=0;
	//Get_3D_pos(point);

	
	CView::OnRButtonDown(nFlags, point);
}

void CPointCloudView::OnRButtonUp(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default

	CView::OnRButtonUp(nFlags, point);
}


void CPointCloudView::ResetViewport()
{

}
void CPointCloudView::ReShape(int x, int y)
{
	glOrtho(-x/2, x/2,-y/2, y/2,-100, 100);
}

void CPointCloudView::InitOpenGL()
{
	CPointCloudDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	/*glOrtho(-pDoc->m_dbDistance,pDoc->m_dbDistance,
		-pDoc->m_dbDistance,pDoc->m_dbDistance,
	-10000,10000);*/
	glOrtho(-1400, 1400,
		-1400, 1400,
		-10000, 10000);
	//背景色 默认为黑色
	glClearColor(0.0f,0.0f,0.0f,1.0f);
	glColor3f( 1.0, 1.0, 1.0);
	
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);//清除缓存将背景置空
	glTranslatef(-1.5f, 0.0f, -6.0f);
	//gluLookAt(0,0,-pDoc->m_dbDistance,0,0,0,0.0, 1.0, 0.0);

	bInitOpenGL=TRUE;

}


//void CPointCloudView::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
//{
//	// TODO:  在此添加消息处理程序代码和/或调用默认值
//	
//	if (nChar == WM_SYSKEYUP)
//	{
//		alt = false;
//		MessageBox("dakai");
//	}
//	
//	CView::OnKeyDown(nChar, nRepCnt, nFlags);
//}
//BOOL  CPointCloudView::PreTranslateMessage(MSG* pMsg)
//{
//
//	
//	
//
//	return true;
//}

//void CPointCloudView::OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags)
//{
//
//	// TODO:  在此添加消息处理程序代码和/或调用默认值
//	if (nChar == WM_SYSKEYUP)
//	{
//		alt = false;
//		MessageBox("dakai");
//	}
//	
//	CView::OnKeyUp(nChar, nRepCnt, nFlags);
//}


//void CPointCloudView::OnPaint()
//{
//	CPaintDC dc(this); // device context for painting
//	// TODO:  在此处添加消息处理程序代码
//	// 不为绘图消息调用 CView::OnPaint()
//}



//监听alt键不能单纯用可以用keydown来解决监听问题   用消息监听函数 WM_SYSKEYUP或者用onsyskeydown来解决

void CPointCloudView::OnSysKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	// TODO:  在此添加消息处理程序代码和/或调用默认值

	if (nChar == VK_MENU)
		alt = true;
	CView::OnSysKeyDown(nChar, nRepCnt, nFlags);
}


void CPointCloudView::OnSysKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	// TODO:  在此添加消息处理程序代码和/或调用默认值
	if (nChar == VK_MENU)
		alt = false;

	CView::OnSysKeyUp(nChar, nRepCnt, nFlags);
}
