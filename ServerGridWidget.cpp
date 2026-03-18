#include "ServerGridWidget.h"
#include <QVBoxLayout>
#include <QResizeEvent>
#include <QEvent>

bool ServerGridWidget::eventFilter(QObject *obj, QEvent *event)
{
    return QWidget::eventFilter(obj, event);
}

ServerGridWidget::ServerGridWidget(QWidget *parent)
    : QWidget(parent)
    , m_columns(2)
{
    setupUi();
}

ServerGridWidget::~ServerGridWidget()
{
}

void ServerGridWidget::setupUi()
{
    m_scrollArea = new QScrollArea(this);
    m_scrollArea->setWidgetResizable(true);
    m_scrollArea->setAlignment(Qt::AlignTop | Qt::AlignLeft);

    m_containerWidget = new QWidget(this);
    m_scrollArea->setWidget(m_containerWidget);

    m_gridLayout = new QGridLayout(m_containerWidget);
    m_gridLayout->setSpacing(16);
    m_gridLayout->setContentsMargins(16, 16, 16, 16);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->addWidget(m_scrollArea);
}

void ServerGridWidget::addServer(const QString &name, int latency, const QString &protocol, const QString &flagPath, bool connected)
{
    ServerCardWidget *card = new ServerCardWidget(m_containerWidget);
    card->setNodeInfo(name, latency, protocol, connected);
    card->setFlag(flagPath);

    connect(card, &ServerCardWidget::clicked, this, &ServerGridWidget::onCardClicked);
    connect(card, &ServerCardWidget::toggled, this, &ServerGridWidget::onCardToggled);

    int row = m_cards.size() / m_columns;
    int col = m_cards.size() % m_columns;
    m_gridLayout->addWidget(card, row, col);

    m_cards.append(card);
}

void ServerGridWidget::clearServers()
{
    for (ServerCardWidget *card : m_cards) {
        m_gridLayout->removeWidget(card);
        delete card;
    }
    m_cards.clear();
}

int ServerGridWidget::serverCount() const
{
    return m_cards.size();
}

ServerCardWidget* ServerGridWidget::getServer(int index) const
{
    if (index >= 0 && index < m_cards.size()) {
        return m_cards[index];
    }
    return nullptr;
}

void ServerGridWidget::onCardClicked()
{
    ServerCardWidget *card = qobject_cast<ServerCardWidget*>(sender());
    if (card) {
        int index = m_cards.indexOf(card);
        if (index != -1) {
            emit serverClicked(index);
        }
    }
}

void ServerGridWidget::onCardToggled(bool checked)
{
    ServerCardWidget *card = qobject_cast<ServerCardWidget*>(sender());
    if (card) {
        int index = m_cards.indexOf(card);
        if (index != -1) {
            emit serverToggled(index, checked);
        }
    }
}

void ServerGridWidget::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);

    int width = m_containerWidget->width() - 32;
    int cardWidth = 280;
    int newColumns = qMax(1, width / cardWidth);

    if (newColumns != m_columns) {
        m_columns = newColumns;

        QList<ServerCardWidget*> cards = m_cards;
        while (m_gridLayout->count() > 0) {
            QLayoutItem *item = m_gridLayout->takeAt(0);
            if (item->widget()) {
                m_gridLayout->removeWidget(item->widget());
            }
            delete item;
        }

        for (int i = 0; i < cards.size(); ++i) {
            int row = i / m_columns;
            int col = i % m_columns;
            m_gridLayout->addWidget(cards[i], row, col);
        }
    }
}
