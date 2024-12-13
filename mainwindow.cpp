#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QStandardItemModel>
#include <QGraphicsEllipseItem>
#include <QGraphicsTextItem>
#include <QPixmap>
#include <QGraphicsPixmapItem>
#include <cmath>

QMap<QString, QPointF> countryCoordinates = {
    {"Belgium", QPointF(500, 110)},
    {"Chile", QPointF(290, 360)},
    {"Denmark", QPointF(505, 92)},
    {"Italy", QPointF(516, 144)},
    {"Norway", QPointF(507, 73)},
    {"Sweden", QPointF(522, 73)},
    {"Spain", QPointF(472, 148)},
    {"USA", QPointF(200, 150)},
    {"Canada", QPointF(200, 80)},
    {"Mexico", QPointF(215, 205)},
    {"Brazil", QPointF(350, 300)},
    {"Argentina", QPointF(300, 350)},
    {"UK", QPointF(480, 100)},
    {"Germany", QPointF(510, 110)},
    {"China", QPointF(750, 170)},
    {"India", QPointF(690, 200)},
    {"Australia", QPointF(840, 320)},
    {"South Africa", QPointF(543, 343)},
    {"Japan", QPointF(850, 158)},
    {"Egypt", QPointF(559, 188)},
    {"France", QPointF(488, 131)}
};



void MainWindow::displayMap(QMap<QString, QMap<QString, double>> &data)
{
    QGraphicsScene *scene = new QGraphicsScene(this);

    // Загрузка фоновой карты
    QPixmap mapPixmap("C:\\Qt\\Qt5.12.2\\Projects\\SalesAnalytics\\world_map.png");
    QGraphicsPixmapItem *mapItem = scene->addPixmap(mapPixmap);
    mapItem->setZValue(-1); // Фон на задний план

    // Расставляем страны
    for (const auto &country : data.keys()) {
        if (!countryCoordinates.contains(country)) {
            qDebug() << "Missing coordinates for country:" << country;
            continue;
        }

        QPointF coords = countryCoordinates[country];
        const auto &genres = data[country];

        // Считаем общий объём продаж
        double totalSales = 0;
        for (const auto &sales : genres.values()) {
            totalSales += sales;
        }

        // Цвет и размер зависят от продаж
        QColor color = QColor::fromHsv(0, 255, std::min(255.0, totalSales));
        double radius = sqrt(totalSales)*2;

        // Круг для страны
        QGraphicsEllipseItem *circle = scene->addEllipse(
            coords.x() - radius / 2, coords.y() - radius / 2,
            radius, radius,
            QPen(Qt::NoPen), QBrush(color)
        );

        // Текстовая подпись
        QGraphicsTextItem *textItem = scene->addText(country);
        textItem->setPos(coords.x() + radius / 2, coords.y() + radius / 2);
    }

    ui->graphicsView->setScene(scene);
    ui->graphicsView->show();
}



MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // Подключение к базе данных
    db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName("C:\\Qt\\Qt5.12.2\\Projects\\SalesAnalytics\\chinook.db");
    if (!db.open()) {
        qDebug() << "Database error:" << db.lastError().text();
        return;
    }

    // Подключаем кнопки к слотам
    connect(ui->btnMonthlySales, &QPushButton::clicked, this, &MainWindow::showMonthlySales);
    connect(ui->btnRevenueByGenre, &QPushButton::clicked, this, &MainWindow::showRevenueByGenre);
    connect(ui->btnTop3Artists, &QPushButton::clicked, this, &MainWindow::showTop3ArtistsByGenre);
    connect(ui->btnTop5Artists, &QPushButton::clicked, this, &MainWindow::showTop5ArtistsOverall);
    connect(ui->btnInteractiveMap, &QPushButton::clicked, this, &MainWindow::showInteractiveMap);
}

MainWindow::~MainWindow()
{
    delete ui;
    db.close();
}

void MainWindow::displayTable(QSqlQuery query, const QStringList &headers)
{
    QStandardItemModel *model = new QStandardItemModel(this);
    int columnCount = headers.size();
    model->setColumnCount(columnCount);

    // Устанавливаем заголовки
    for (int i = 0; i < columnCount; ++i) {
        model->setHeaderData(i, Qt::Horizontal, headers[i]);
    }

    // Заполняем модель данными
    int row = 0;
    while (query.next()) {
        for (int col = 0; col < columnCount; ++col) {
            model->setItem(row, col, new QStandardItem(query.value(col).toString()));
        }
        ++row;
    }

    ui->tableView->setModel(model);
    ui->tableView->resizeColumnsToContents();
}

void MainWindow::showMonthlySales()
{
    QSqlQuery query(db);
    query.exec(R"(
        SELECT strftime('%Y-%m', invoices.InvoiceDate) AS Month, SUM(invoice_items.Quantity) AS TotalSales
        FROM invoice_items
        JOIN invoices ON invoice_items.InvoiceId = invoices.InvoiceId
        GROUP BY Month
        ORDER BY Month;
    )");
    displayTable(query, {"Month", "Total Sales"});
}

void MainWindow::showRevenueByGenre()
{
    QSqlQuery query(db);
    query.exec(R"(
        SELECT genres.Name AS GenreName, ROUND(SUM(invoice_items.Quantity * invoice_items.UnitPrice), 2) AS Revenue
        FROM invoice_items
        JOIN tracks ON invoice_items.TrackId = tracks.TrackId
        JOIN genres ON tracks.GenreId = genres.GenreId
        GROUP BY genres.GenreId
        ORDER BY Revenue DESC;
    )");
    displayTable(query, {"Genre", "Revenue"});
}

void MainWindow::showTop3ArtistsByGenre()
{
    QSqlQuery query(db);
    query.exec(R"(
        SELECT GenreName, ArtistName, TotalSales
        FROM (
            SELECT genres.Name AS GenreName, artists.Name AS ArtistName,
                   SUM(invoice_items.Quantity) AS TotalSales,
                   RANK() OVER (PARTITION BY genres.GenreId ORDER BY SUM(invoice_items.Quantity) DESC) AS Rank
            FROM invoice_items
            JOIN tracks ON invoice_items.TrackId = tracks.TrackId
            JOIN albums ON tracks.AlbumId = albums.AlbumId
            JOIN artists ON albums.ArtistId = artists.ArtistId
            JOIN genres ON tracks.GenreId = genres.GenreId
            GROUP BY genres.GenreId, artists.ArtistId
        )
        WHERE Rank <= 3
        ORDER BY GenreName, Rank;
    )");
    displayTable(query, {"Genre", "Artist", "Total Sales"});
}

void MainWindow::showTop5ArtistsOverall()
{
    QSqlQuery query(db);
    query.exec(R"(
        SELECT artists.Name AS ArtistName, SUM(invoice_items.Quantity) AS TotalSales
        FROM invoice_items
        JOIN tracks ON invoice_items.TrackId = tracks.TrackId
        JOIN albums ON tracks.AlbumId = albums.AlbumId
        JOIN artists ON albums.ArtistId = artists.ArtistId
        GROUP BY artists.ArtistId
        ORDER BY TotalSales DESC
        LIMIT 5;
    )");
    displayTable(query, {"Artist", "Total Sales"});
}
void MainWindow::showInteractiveMap()
{
    QSqlQuery query(db);
    query.exec(R"(
        SELECT BillingCountry, genres.Name AS GenreName, SUM(invoice_items.Quantity) AS TotalSales
        FROM invoice_items
        JOIN invoices ON invoice_items.InvoiceId = invoices.InvoiceId
        JOIN tracks ON invoice_items.TrackId = tracks.TrackId
        JOIN genres ON tracks.GenreId = genres.GenreId
        GROUP BY BillingCountry, genres.GenreId
        ORDER BY BillingCountry, TotalSales DESC;
    )");

    QMap<QString, QMap<QString, double>> mapData; // Map<Country, Map<Genre, Sales>>

    while (query.next()) {
        QString country = query.value("BillingCountry").toString();
        QString genre = query.value("GenreName").toString();
        double sales = query.value("TotalSales").toDouble();

        mapData[country][genre] += sales;
    }

    displayMap(mapData); // Передаём данные для отображения
}
