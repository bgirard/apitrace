#include "apitracemodel.h"

#include "apitracecall.h"
#include "loaderthread.h"
#include "trace_parser.hpp"

#include <QDebug>
#include <QImage>
#include <QVariant>


ApiTraceModel::ApiTraceModel(QObject *parent)
    : QAbstractItemModel(parent),
      m_trace(0)
{
}

ApiTraceModel::~ApiTraceModel()
{
    m_trace = 0;
}

QVariant ApiTraceModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (index.column() != 0)
        return QVariant();

    ApiTraceEvent *itm = item(index);
    if (!itm) {
        return QVariant();
    }

    switch (role) {
    case Qt::DisplayRole:
        return itm->staticText().text();
    case Qt::DecorationRole:
        return QImage();
    case Qt::ToolTipRole: {
        const QString stateText = tr("State info available.");
        if (itm->type() == ApiTraceEvent::Call) {
            ApiTraceCall *call = static_cast<ApiTraceCall*>(itm);
            if (call->state().isEmpty())
                return QString::fromLatin1("%1)&nbsp;<b>%2</b>")
                    .arg(call->index)
                    .arg(call->name);
            else
                return QString::fromLatin1("%1)&nbsp;<b>%2</b><br/>%3")
                    .arg(call->index)
                    .arg(call->name)
                    .arg(stateText);
        } else {
            ApiTraceFrame *frame = static_cast<ApiTraceFrame*>(itm);
            QString text = frame->staticText().text();
            if (frame->state().isEmpty())
                return QString::fromLatin1("<b>%1</b>").arg(text);
            else
                return QString::fromLatin1("<b>%1</b><br/>%2")
                    .arg(text)
                    .arg(stateText);
        }
    }
    case ApiTraceModel::EventRole:
        return QVariant::fromValue(itm);
    }

    return QVariant();
}

Qt::ItemFlags ApiTraceModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return 0;

    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

QVariant ApiTraceModel::headerData(int section, Qt::Orientation orientation,
                                   int role ) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        switch (section) {
        case 0:
            return tr("Events");
        case 1:
            return tr("Flags");
        default:
            //fall through
            break;
        }
    }

    return QVariant();
}

QModelIndex ApiTraceModel::index(int row, int column,
                                 const QModelIndex &parent) const
{
    if (parent.isValid() && parent.column() != 0)
        return QModelIndex();

    ApiTraceEvent *event = item(parent);
    if (event) {
        if (event->type() != ApiTraceEvent::Frame) {
            qDebug()<<"got a valid parent but it's not a frame "<<event->type();
            return QModelIndex();
        }
        ApiTraceFrame *frame = static_cast<ApiTraceFrame*>(event);
        ApiTraceCall *call = frame->calls.value(row);
        if (call)
            return createIndex(row, column, call);
        else
            return QModelIndex();
    } else {
        ApiTraceFrame *frame = m_trace->frameAt(row);
        if (frame)
            return createIndex(row, column, frame);
        else
            return QModelIndex();
    }
    return QModelIndex();
}

bool ApiTraceModel::hasChildren(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        ApiTraceEvent *event = item(parent);
        if (event && event->type() == ApiTraceEvent::Frame) {
            ApiTraceFrame *frame = static_cast<ApiTraceFrame*>(event);
            return !frame->calls.isEmpty();
        } else
            return false;
    } else {
        return (rowCount() > 0);
    }
}

QModelIndex ApiTraceModel::parent(const QModelIndex &index) const
{
    if (!index.isValid())
        return QModelIndex();

    ApiTraceEvent *event = item(index);
    if (event && event->type() == ApiTraceEvent::Call) {
        ApiTraceCall *call = static_cast<ApiTraceCall*>(event);
        Q_ASSERT(call->parentFrame);
        return createIndex(call->parentFrame->number,
                           0, call->parentFrame);
    }
    return QModelIndex();
}

int ApiTraceModel::rowCount(const QModelIndex &parent) const
{
    if (!parent.isValid())
        return m_trace->numFrames();

    ApiTraceEvent *event = item(parent);
    if (!event || event->type() == ApiTraceEvent::Call)
        return 0;

    ApiTraceFrame *frame = static_cast<ApiTraceFrame*>(event);
    if (frame)
        return frame->calls.count();

    return 0;
}

int ApiTraceModel::columnCount(const QModelIndex &parent) const
{
    return 1;
}

bool ApiTraceModel::insertRows(int position, int rows,
                               const QModelIndex &parent)
{
    return false;
}

bool ApiTraceModel::removeRows(int position, int rows,
                               const QModelIndex &parent)
{
    bool success = true;

    Q_UNUSED(parent);

    beginRemoveRows(parent, position, position + rows - 1);
    //XXX remove it from ApiTrace
    endRemoveRows();

    return success;
}

void ApiTraceModel::setApiTrace(ApiTrace *trace)
{
    if (m_trace == trace)
        return;
    if (m_trace)
        disconnect(m_trace);
    m_trace = trace;
    connect(m_trace, SIGNAL(invalidated()),
            this, SLOT(invalidateFrames()));
    connect(m_trace, SIGNAL(framesInvalidated()),
            this, SLOT(invalidateFrames()));
    connect(m_trace, SIGNAL(framesAdded(int, int)),
            this, SLOT(appendFrames(int, int)));
}

const ApiTrace * ApiTraceModel::apiTrace() const
{
    return m_trace;
}

void ApiTraceModel::invalidateFrames()
{
    beginResetModel();
    endResetModel();
}

void ApiTraceModel::appendFrames(int oldCount, int numAdded)
{
    beginInsertRows(QModelIndex(), oldCount,
                    oldCount + numAdded - 1);
    endInsertRows();
}

ApiTraceEvent * ApiTraceModel::item(const QModelIndex &index) const
{
    if (!index.isValid())
        return 0;
    return static_cast<ApiTraceEvent*>(index.internalPointer());
}

void ApiTraceModel::stateSetOnEvent(ApiTraceEvent *event)
{
    if (!event)
        return;

    if (event->type() == ApiTraceEvent::Call) {
        ApiTraceCall *call = static_cast<ApiTraceCall*>(event);
        ApiTraceFrame *frame = call->parentFrame;
        int row = frame->calls.indexOf(call);
        QModelIndex index = createIndex(row, 0, call);
        emit dataChanged(index, index);
    } else if (event->type() == ApiTraceEvent::Frame) {
        ApiTraceFrame *frame = static_cast<ApiTraceFrame*>(event);
        const QList<ApiTraceFrame*> frames = m_trace->frames();
        int row = frames.indexOf(frame);
        QModelIndex index = createIndex(row, 0, frame);
        emit dataChanged(index, index);
    }
}

QModelIndex ApiTraceModel::callIndex(int callNum) const
{
    ApiTraceCall *call = m_trace->callWithIndex(callNum);
    return indexForCall(call);
}

QModelIndex ApiTraceModel::indexForCall(ApiTraceCall *call) const
{
    if (!call) {
        return QModelIndex();
    }

    ApiTraceFrame *frame = call->parentFrame;
    Q_ASSERT(frame);

    int row = frame->calls.indexOf(call);
    if (row < 0) {
        qDebug() << "Couldn't find call num "<<call->index<<" inside parent!";
        return QModelIndex();
    }
    return createIndex(row, 0, call);
}

#include "apitracemodel.moc"
