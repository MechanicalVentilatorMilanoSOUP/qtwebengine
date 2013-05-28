#include "render_widget_host_view_qt.h"

#include "backing_store_qt.h"
#include "web_event_factory.h"
#include "raster_window.h"

#include "content/browser/renderer_host/render_view_host_impl.h"
#include "content/common/gpu/gpu_messages.h"
#include "raster_window.h"
#include "third_party/WebKit/Source/WebKit/chromium/public/WebScreenInfo.h"

#include <QEvent>
#include <QFocusEvent>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QScreen>

#define QT_NOT_YET_IMPLEMENTED fprintf(stderr, "function %s not implemented! - %s:%d\n", __func__, __FILE__, __LINE__);

static void GetScreenInfoFromNativeWindow(QWindow* window, WebKit::WebScreenInfo* results)
{
    QScreen* screen = window->screen();

    WebKit::WebScreenInfo r;
    r.deviceScaleFactor = screen->devicePixelRatio();
    r.depthPerComponent = 8;
    r.depth = screen->depth();
    r.isMonochrome = (r.depth == 1);

    QRect virtualGeometry = screen->virtualGeometry();
    r.rect = WebKit::WebRect(virtualGeometry.x(), virtualGeometry.y(), virtualGeometry.width(), virtualGeometry.height());
    QRect available = screen->availableGeometry();
    r.availableRect = WebKit::WebRect(available.x(), available.y(), available.width(), available.height());
    *results = r;
}

namespace content {

RenderWidgetHostView* RenderWidgetHostView::CreateViewForWidget(
    RenderWidgetHost* widget) {
  return new RenderWidgetHostViewQt(widget);
}

// static
void RenderWidgetHostViewPort::GetDefaultScreenInfo(WebKit::WebScreenInfo* results) {
    QWindow dummy;
    GetScreenInfoFromNativeWindow(&dummy, results);
}

RenderWidgetHostViewQt::RenderWidgetHostViewQt(content::RenderWidgetHost* widget)
    : m_host(content::RenderWidgetHostImpl::From(widget))
    , m_view(0)
{
    m_host->SetView(this);
}

RenderWidgetHostViewQt::~RenderWidgetHostViewQt()
{
}

bool RenderWidgetHostViewQt::handleEvent(QEvent* event) {

    switch(event->type()) {
    case QEvent::MouseButtonDblClick:
    case QEvent::MouseButtonPress:
    case QEvent::MouseButtonRelease:
    case QEvent::MouseMove:
        handleMouseEvent(static_cast<QMouseEvent*>(event));
        break;
    case QEvent::KeyPress:
    case QEvent::KeyRelease:
        handleKeyEvent(static_cast<QKeyEvent*>(event));
        break;
    case QEvent::Wheel:
        handleWheelEvent(static_cast<QWheelEvent*>(event));
        break;
    case QEvent::FocusIn:
    case QEvent::FocusOut:
        handleFocusEvent(static_cast<QFocusEvent*>(event));
        break;
    default:
        return false;
    }
    return true;
}

content::BackingStore *RenderWidgetHostViewQt::AllocBackingStore(const gfx::Size &size)
{
    if (m_view)
        return new BackingStoreQt(m_host, size, new QWindow);
    return 0;
}

RenderWidgetHostView* RenderWidgetHostViewQt::CreateViewForWidget(content::RenderWidgetHost* widget)
{
    return new RenderWidgetHostViewQt(widget);
}

void RenderWidgetHostViewQt::InitAsChild(gfx::NativeView parent_view)
{
    m_view = new RasterWindow(this);
    bool force_create = !m_host->empty();
    BackingStoreQt* backing_store = static_cast<BackingStoreQt*>(m_host->GetBackingStore(force_create));
    m_view->setBackingStore(backing_store);

    RasterWindowContainer* container = reinterpret_cast<RasterWindowContainer*>(parent_view);
    container->insert(m_view);
}

void RenderWidgetHostViewQt::InitAsPopup(content::RenderWidgetHostView*, const gfx::Rect&)
{
    m_view = new RasterWindow(this);
}

void RenderWidgetHostViewQt::InitAsFullscreen(content::RenderWidgetHostView*)
{
    m_view = new RasterWindow(this);
}

content::RenderWidgetHost* RenderWidgetHostViewQt::GetRenderWidgetHost() const
{
    return m_host;
}

void RenderWidgetHostViewQt::SetSize(const gfx::Size& size)
{
    int width = size.width();
    int height = size.height();
    // int width = std::min(size.width(), kMaxWindowWidth);
    // int height = std::min(size.height(), kMaxWindowHeight);
    if (IsPopup()) {
        // We're a popup, honor the size request.
        m_view->resize(width,height);
    }

    if (m_requestedSize.width() != width ||
        m_requestedSize.height() != height) {
        m_requestedSize = gfx::Size(width, height);
        // m_host->SendScreenRects();
        m_host->WasResized();
    }
}

void RenderWidgetHostViewQt::SetBounds(const gfx::Rect& rect)
{
    // This is called when webkit has sent us a Move message.
    if (IsPopup())
        m_view->setGeometry(rect.x(), rect.y(), rect.width(), rect.height());

    SetSize(rect.size());
}

// FIXME: Should this really return a QWindow pointer?
gfx::NativeView RenderWidgetHostViewQt::GetNativeView() const
{
    QT_NOT_YET_IMPLEMENTED
    // return m_view;
    return gfx::NativeView();
}

RasterWindow* RenderWidgetHostViewQt::GetNativeViewQt() const
{
    return m_view;
}

gfx::NativeViewId RenderWidgetHostViewQt::GetNativeViewId() const
{
    QT_NOT_YET_IMPLEMENTED
    return gfx::NativeViewId();
}

gfx::NativeViewAccessible RenderWidgetHostViewQt::GetNativeViewAccessible()
{
    NOTIMPLEMENTED();
    return NULL;
}

// Set focus to the associated View component.
void RenderWidgetHostViewQt::Focus()
{
    m_view->setFocus(Qt::MouseFocusReason);
}

bool RenderWidgetHostViewQt::HasFocus() const
{
    return m_view->hasFocus();
}

bool RenderWidgetHostViewQt::IsSurfaceAvailableForCopy() const
{
    return true;
}

void RenderWidgetHostViewQt::Show()
{
    m_view->show();
}

void RenderWidgetHostViewQt::Hide()
{
    m_view->hide();
}

bool RenderWidgetHostViewQt::IsShowing()
{
    return m_view->isVisible();
}

// Retrieve the bounds of the View, in screen coordinates.
gfx::Rect RenderWidgetHostViewQt::GetViewBounds() const
{
    QRect rect = m_view->geometry();
    QPoint screenPos = m_view->mapToGlobal(QPoint(0,0));

    return gfx::Rect(screenPos.x(), screenPos.y(), rect.width(), rect.height());
}

// Subclasses should override this method to do what is appropriate to set
// the custom background for their platform.
void RenderWidgetHostViewQt::SetBackground(const SkBitmap& background)
{
    RenderWidgetHostViewBase::SetBackground(background);
    // Send(new ViewMsg_SetBackground(m_host->GetRoutingID(), background));
}

// Return value indicates whether the mouse is locked successfully or not.
bool RenderWidgetHostViewQt::LockMouse()
{
    QT_NOT_YET_IMPLEMENTED
    return false;
}
void RenderWidgetHostViewQt::UnlockMouse()
{
    QT_NOT_YET_IMPLEMENTED
}

// Returns true if the mouse pointer is currently locked.
bool RenderWidgetHostViewQt::IsMouseLocked()
{
    QT_NOT_YET_IMPLEMENTED
    return false;
}

// FIXME: remove TOOLKIT_GTK related things.
#if defined(TOOLKIT_GTK)
// Gets the event for the last mouse down.
GdkEventButton* RenderWidgetHostViewQt::GetLastMouseDown()
{
    return 0;
}

// Builds a submenu containing all the gtk input method commands.
gfx::NativeView RenderWidgetHostViewQt::BuildInputMethodsGtkMenu()
{
}
#endif  // defined(TOOLKIT_GTK)

void RenderWidgetHostViewQt::WasShown()
{
    if (m_view->isVisible())
        return;

    m_host->WasShown();
}

void RenderWidgetHostViewQt::WasHidden()
{
    if (!m_view->isVisible())
        return;

    m_host->WasHidden();
}

void RenderWidgetHostViewQt::MovePluginWindows(const gfx::Vector2d&, const std::vector<webkit::npapi::WebPluginGeometry>&)
{
    QT_NOT_YET_IMPLEMENTED
}

void RenderWidgetHostViewQt::Blur()
{
    m_host->Blur();
}

void RenderWidgetHostViewQt::UpdateCursor(const WebCursor&)
{
    QT_NOT_YET_IMPLEMENTED
}

void RenderWidgetHostViewQt::SetIsLoading(bool)
{
    QT_NOT_YET_IMPLEMENTED
    // Give visual feedback for loading process.
}

void RenderWidgetHostViewQt::TextInputStateChanged(const ViewHostMsg_TextInputState_Params&)
{
    QT_NOT_YET_IMPLEMENTED
}

void RenderWidgetHostViewQt::ImeCancelComposition()
{
    QT_NOT_YET_IMPLEMENTED
}

void RenderWidgetHostViewQt::ImeCompositionRangeChanged(const ui::Range&, const std::vector<gfx::Rect>&)
{
    // FIXME: not implemented?
    QT_NOT_YET_IMPLEMENTED
}

void RenderWidgetHostViewQt::DidUpdateBackingStore(const gfx::Rect& scroll_rect, const gfx::Vector2d& scroll_delta, const std::vector<gfx::Rect>& copy_rects)
{
    if (!m_view->isVisible())
        return;

    Paint(scroll_rect);

    for (size_t i = 0; i < copy_rects.size(); ++i) {
        gfx::Rect rect = gfx::SubtractRects(copy_rects[i], scroll_rect);
        if (rect.IsEmpty())
            continue;

        Paint(rect);
    }
}

void RenderWidgetHostViewQt::RenderViewGone(base::TerminationStatus, int)
{
    Destroy();
}

void RenderWidgetHostViewQt::Destroy()
{
    delete m_view;
    m_view = 0;
}

void RenderWidgetHostViewQt::SetTooltipText(const string16&)
{
    QT_NOT_YET_IMPLEMENTED
}

void RenderWidgetHostViewQt::SelectionBoundsChanged(const ViewHostMsg_SelectionBounds_Params&)
{
    QT_NOT_YET_IMPLEMENTED
}

void RenderWidgetHostViewQt::ScrollOffsetChanged()
{
    // FIXME: not implemented?
}

void RenderWidgetHostViewQt::CopyFromCompositingSurface(const gfx::Rect& src_subrect, const gfx::Size& /* dst_size */, const base::Callback<void(bool, const SkBitmap&)>& callback)
{
    // Grab the snapshot from the renderer as that's the only reliable way to
    // readback from the GPU for this platform right now.
    // FIXME: is this true?
    GetRenderWidgetHost()->GetSnapshotFromRenderer(src_subrect, callback);
}

void RenderWidgetHostViewQt::CopyFromCompositingSurfaceToVideoFrame(const gfx::Rect& src_subrect, const scoped_refptr<media::VideoFrame>& target, const base::Callback<void(bool)>& callback)
{
    NOTIMPLEMENTED();
    callback.Run(false);
}

bool RenderWidgetHostViewQt::CanCopyToVideoFrame() const
{
    return false;
}

void RenderWidgetHostViewQt::OnAcceleratedCompositingStateChange()
{
    // bool activated = m_host->is_accelerated_compositing_active();
    QT_NOT_YET_IMPLEMENTED
}

void RenderWidgetHostViewQt::AcceleratedSurfaceBuffersSwapped(const GpuHostMsg_AcceleratedSurfaceBuffersSwapped_Params& params, int gpu_host_id)
{
    AcceleratedSurfaceMsg_BufferPresented_Params ack_params;
    ack_params.sync_point = 0;
    content::RenderWidgetHostImpl::AcknowledgeBufferPresent(params.route_id, gpu_host_id, ack_params);
}

void RenderWidgetHostViewQt::AcceleratedSurfacePostSubBuffer(const GpuHostMsg_AcceleratedSurfacePostSubBuffer_Params& params, int gpu_host_id)
{
    AcceleratedSurfaceMsg_BufferPresented_Params ack_params;
    ack_params.sync_point = 0;
    content::RenderWidgetHostImpl::AcknowledgeBufferPresent(params.route_id, gpu_host_id, ack_params);
}

void RenderWidgetHostViewQt::AcceleratedSurfaceSuspend()
{
    //FIXME: not implemented?
}

void RenderWidgetHostViewQt::AcceleratedSurfaceRelease()
{
    //FIXME: not implemented?
}

bool RenderWidgetHostViewQt::HasAcceleratedSurface(const gfx::Size&)
{
    return false;
}

void RenderWidgetHostViewQt::GetScreenInfo(WebKit::WebScreenInfo* results)
{
    QWindow* window = m_view->window()->windowHandle();
    if (!window)
        return;
    GetScreenInfoFromNativeWindow(window, results);
}

gfx::Rect RenderWidgetHostViewQt::GetBoundsInRootWindow()
{
    QRect r = m_view->frameGeometry();
    return gfx::Rect(r.x(), r.y(), r.width(), r.height());
}

gfx::GLSurfaceHandle RenderWidgetHostViewQt::GetCompositingSurface()
{
    QT_NOT_YET_IMPLEMENTED
    return gfx::GLSurfaceHandle();
}

void RenderWidgetHostViewQt::SetHasHorizontalScrollbar(bool) { }

void RenderWidgetHostViewQt::SetScrollOffsetPinning(bool, bool) { }

void RenderWidgetHostViewQt::OnAccessibilityNotifications(const std::vector<AccessibilityHostMsg_NotificationParams>&)
{
    QT_NOT_YET_IMPLEMENTED
}

void RenderWidgetHostViewQt::Paint(const gfx::Rect& scroll_rect)
{
    bool force_create = !m_host->empty();
    BackingStoreQt* backing_store = static_cast<BackingStoreQt*>(m_host->GetBackingStore(force_create));
    if (backing_store && m_view) {
        QSize s = m_view->size();
        QRect rect(0, 0, s.width(), s.height());
        m_view->setBackingStore(backing_store);
        m_view->update();
    }
}

bool RenderWidgetHostViewQt::IsPopup() const
{
    return popup_type_ != WebKit::WebPopupTypeNone;
}

void RenderWidgetHostViewQt::handleMouseEvent(QMouseEvent* ev)
{
    m_host->ForwardMouseEvent(WebEventFactory::toWebMouseEvent(ev));
}

void RenderWidgetHostViewQt::handleKeyEvent(QKeyEvent *ev)
{
    m_host->ForwardKeyboardEvent(WebEventFactory::toWebKeyboardEvent(ev));
}

void RenderWidgetHostViewQt::handleWheelEvent(QWheelEvent *ev)
{
    m_host->ForwardWheelEvent(WebEventFactory::toWebWheelEvent(ev));
}

void RenderWidgetHostViewQt::handleFocusEvent(QFocusEvent *ev)
{
    if (ev->gotFocus()) {
        m_host->GotFocus();
        m_host->SetActive(true);
        ev->accept();
    } else if (ev->lostFocus()) {
        m_host->SetActive(false);
        m_host->Blur();
        ev->accept();
    }
}

}

