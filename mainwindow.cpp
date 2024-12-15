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
    connect(ui->btnMonthlySales, &QPushButton::clicked, this, &MainWindow::showMonthlySalesChart);

    connect(ui->btnRevenueByGenre, &QPushButton::clicked, this, &MainWindow::showRevenueByGenre);
    connect(ui->btnRevenueByGenre, &QPushButton::clicked, this, &MainWindow::showRevenueByGenreChart);

    connect(ui->btnTop3Artists, &QPushButton::clicked, this, &MainWindow::showTop3ArtistsByGenre);
    connect(ui->btnTop3Artists, &QPushButton::clicked, this, &MainWindow::showTop3ArtistsByGenreChart);

    connect(ui->btnTop5Artists, &QPushButton::clicked, this, &MainWindow::showTop5ArtistsOverall);
    connect(ui->btnTop5Artists, &QPushButton::clicked, this, &MainWindow::showTop5ArtistsPentagonChart);
    connect(ui->btnTop5Artists, &QPushButton::clicked, this, &MainWindow::showTop5ArtistsChart);

    connect(ui->btnInteractiveMap, &QPushButton::clicked, this, &MainWindow::showInteractiveMap);
}

MainWindow::~MainWindow()
{
    delete ui;
    db.close();
}

void MainWindow::clearScene()
{
    if (ui->graphicsView->scene()) {
        ui->graphicsView->scene()->clear();
        ui->graphicsView->viewport()->update();
    }
    if (ui->chartView->scene()) {
        ui->chartView->scene()->clear();
        ui->chartView->viewport()->update();
    }
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
    displayTable(query, {"D", "Total sales"});
}

void MainWindow::showMonthlySalesChart()
{
    QSqlQuery query(db);
    query.exec(R"(
        SELECT strftime('%Y', invoices.InvoiceDate) AS Year,
               strftime('%m', invoices.InvoiceDate) AS Month,
               SUM(invoice_items.Quantity) AS TotalSales
        FROM invoice_items
        JOIN invoices ON invoice_items.InvoiceId = invoices.InvoiceId
        GROUP BY Year, Month
        ORDER BY Year, Month;
    )");

    QBarSeries *series = new QBarSeries();
    QMap<QString, QBarSet*> yearSets; // QBarSet для каждого года
    QStringList months;

    // Инициализация месяцев (от 1 до 12)
    for (int i = 1; i <= 12; ++i) {
        months << QString::number(i);
    }

    // Добавляем значения в QBarSet для каждого года
    while (query.next()) {
        QString year = query.value("Year").toString();
        int month = query.value("Month").toInt();
        double sales = query.value("TotalSales").toDouble();

        // Создаём QBarSet для года, если его ещё нет
        if (!yearSets.contains(year)) {
            yearSets[year] = new QBarSet(year);
            for (int i = 0; i < 12; ++i) {
                yearSets[year]->append(0); // Заполняем нулями все месяцы
            }
            series->append(yearSets[year]);
        }

        // Добавляем продажи в соответствующий месяц
        yearSets[year]->replace(month - 1, sales);
    }

    // Настраиваем график
    QChart *chart = new QChart();
    chart->addSeries(series);
    chart->setTitle("Monthly Sales by Year");
    chart->setAnimationOptions(QChart::SeriesAnimations);

    // Ось X: месяца от 1 до 12
    QBarCategoryAxis *axisX = new QBarCategoryAxis();
    axisX->append(months);
    axisX->setTitleText("Month");
    chart->addAxis(axisX, Qt::AlignBottom);
    series->attachAxis(axisX);

    // Ось Y: продажи
    QValueAxis *axisY = new QValueAxis();
    axisY->setTitleText("Total Sales");
    chart->addAxis(axisY, Qt::AlignLeft);
    series->attachAxis(axisY);

    // Легенда с годами
    chart->legend()->setVisible(true);
    chart->legend()->setAlignment(Qt::AlignBottom);

    // Устанавливаем график на chartView
    ui->chartView->setChart(chart);
    ui->chartView->setRenderHint(QPainter::Antialiasing);
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

void MainWindow::showRevenueByGenreChart()
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

    // Создаём круговую диаграмму
    QPieSeries *series = new QPieSeries();

    double otherRevenue = 0.0; // Для суммирования мелких сегментов
    int count = 0;             // Счётчик сегментов

    while (query.next()) {
        QString genreName = query.value("GenreName").toString();
        double revenue = query.value("Revenue").toDouble();

        if (count < 10) { // Добавляем первые 10 сегментов
            series->append(genreName, revenue);
        } else { // Остальные добавляем в категорию "Other"
            otherRevenue += revenue;
        }
        count++;
    }

    // Добавляем "Other" в диаграмму, если есть оставшиеся данные
    if (otherRevenue > 0.0) {
        series->append("Other", otherRevenue);
    }

    // Настраиваем диаграмму
    QChart *chart = new QChart();
    chart->addSeries(series);
    chart->setTitle("Revenue by Genre (Top 10 + Other)");
    chart->legend()->show();

    // Выделяем сегменты и отображаем метки только для топ-10
    for (auto slice : series->slices()) {
            slice->setLabel(QString("%1: %2").arg(slice->label()).arg(slice->value()));
            slice->setExploded(false);
            slice->setLabelVisible(true);
    }

    // Отображаем диаграмму
    ui->chartView->setChart(chart);
    ui->chartView->setRenderHint(QPainter::Antialiasing);
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


void MainWindow::showTop3ArtistsByGenreChart()
{
    QSqlQuery query(db);
    query.exec(R"(
        SELECT genres.Name AS GenreName, artists.Name AS ArtistName, SUM(invoice_items.Quantity) AS TotalSales
        FROM invoice_items
        JOIN tracks ON invoice_items.TrackId = tracks.TrackId
        JOIN albums ON tracks.AlbumId = albums.AlbumId
        JOIN artists ON albums.ArtistId = artists.ArtistId
        JOIN genres ON tracks.GenreId = genres.GenreId
        GROUP BY genres.Name, artists.Name
        ORDER BY genres.Name, TotalSales DESC;
    )");

    // Сопоставление жанров с их топ-3 артистами
    QMap<QString, QVector<QPair<QString, int>>> genreData;

    while (query.next()) {
        QString genre = query.value("GenreName").toString();
        QString artist = query.value("ArtistName").toString();
        int sales = query.value("TotalSales").toInt();

        if (!genreData.contains(genre)) {
            genreData[genre] = QVector<QPair<QString, int>>();
        }

        // Добавляем только топ-3 артистов для каждого жанра
        if (genreData[genre].size() < 3) {
            genreData[genre].append(qMakePair(artist, sales));
        }
    }

    // Создаём серию для stacked bar chart
    QStackedBarSeries *series = new QStackedBarSeries();
    QStringList categories; // Список жанров для оси X

    // Добавляем данные для каждого жанра
    for (auto it = genreData.begin(); it != genreData.end(); ++it) {
        QString genre = it.key();
        categories << genre;

        QVector<QPair<QString, int>> topArtists = it.value();

        for (int i = 0; i < 3; ++i) {
            QString artistName = (i < topArtists.size()) ? topArtists[i].first : "N/A";
            int sales = (i < topArtists.size()) ? topArtists[i].second : 0;

            // Добавляем сегменты для каждого артиста
            QBarSet *set = nullptr;
            if (series->count() <= i) {
                set = new QBarSet(QString("Top %1").arg(i + 1));
                series->append(set);
            } else {
                set = series->barSets().at(i);
            }
            *set << sales;
        }
    }

    // Настройка графика
    QChart *chart = new QChart();
    chart->addSeries(series);
    chart->setTitle("Top 3 Artists by Genre");
    chart->setAnimationOptions(QChart::SeriesAnimations);

    // Ось X: названия жанров
    QBarCategoryAxis *axisX = new QBarCategoryAxis();
    axisX->append(categories);
    axisX->setTitleText("Genres");
    axisX->setLabelsAngle(-90); // Поворачиваем текст на 90 градусов (вертикально)
    chart->addAxis(axisX, Qt::AlignBottom);
    series->attachAxis(axisX);


    // Ось Y: количество продаж
    QValueAxis *axisY = new QValueAxis();
    axisY->setTitleText("Total Sales");
    chart->addAxis(axisY, Qt::AlignLeft);
    series->attachAxis(axisY);

    // Легенда
    chart->legend()->setVisible(true);
    chart->legend()->setAlignment(Qt::AlignBottom);

    // Отображаем диаграмму
    ui->chartView->setChart(chart);
    ui->chartView->setRenderHint(QPainter::Antialiasing);
}


void MainWindow::showTop5ArtistsOverall()
{
    QSqlQuery query(db);
    query.exec(R"(
        SELECT artists.Name AS ArtistName, SUM(invoice_items.Quantity) AS TotalQuantity, ROUND(SUM(invoice_items.UnitPrice * invoice_items.Quantity), 2) AS TotalSales
        FROM invoice_items
        JOIN tracks ON invoice_items.TrackId = tracks.TrackId
        JOIN albums ON tracks.AlbumId = albums.AlbumId
        JOIN artists ON albums.ArtistId = artists.ArtistId
        GROUP BY artists.ArtistId
        ORDER BY TotalSales DESC
        LIMIT 5;
    )");
    displayTable(query, {"Artist", "Total Quantity", "Total Sales"});
}


void MainWindow::showTop5ArtistsPentagonChart()
{
    QSqlQuery query(db);
    query.exec(R"(
        SELECT artists.Name AS ArtistName, ROUND(SUM(invoice_items.UnitPrice * invoice_items.Quantity), 2) AS Revenue
        FROM invoice_items
        JOIN tracks ON invoice_items.TrackId = tracks.TrackId
        JOIN albums ON tracks.AlbumId = albums.AlbumId
        JOIN artists ON albums.ArtistId = artists.ArtistId
        GROUP BY artists.ArtistId
        ORDER BY Revenue DESC
        LIMIT 5;
    )");

    QVector<QPair<QString, double>> artistData;
    double totalRevenue = 0;

    while (query.next()) {
        QString artistName = query.value("ArtistName").toString();
        double revenue = query.value("Revenue").toDouble();
        artistData.append(qMakePair(artistName, revenue));
        totalRevenue += revenue; // Суммируем для среднего значения
    }

    double averageRevenue = totalRevenue / artistData.size(); // Среднее значение

    QGraphicsScene *scene = new QGraphicsScene(this);
    QPointF center(300, 300);
    double radius = 300;

    QVector<QPointF> maxPoints, avgPoints;

    // Создаём точки для максимального и среднего пятиугольника
    for (int i = 0; i < 5; ++i) {
        double angle = 2 * M_PI * i / 5 - M_PI / 2;

        // Точки для максимального значения (нормализуем по максимальному значению)
        double valueFactorMax = artistData[i].second / artistData[0].second;
        maxPoints.append(center + QPointF(radius * valueFactorMax * cos(angle), radius * valueFactorMax * sin(angle)));

        // Точки для среднего значения (нормализуем по среднему значению)
        double valueFactorAvg = averageRevenue / artistData[0].second;
        avgPoints.append(center + QPointF(radius * valueFactorAvg * cos(angle), radius * valueFactorAvg * sin(angle)));
    }

    // Рисуем полупрозрачный внешний пятиугольник (максимальные значения)
    QPolygonF maxPentagon(maxPoints);
    scene->addPolygon(maxPentagon, QPen(Qt::black, 2), QBrush(QColor(0, 255, 255, 200))); // Полупрозрачная заливка

    // Рисуем яркий голубой внутренний пятиугольник (средние значения)
    QPolygonF avgPentagon(avgPoints);
    scene->addPolygon(avgPentagon, QPen(Qt::blue, 2), QBrush(QColor(0, 255, 255, 50))); // Яркая голубая заливка

    // Добавляем подписи для вершин
    for (int i = 0; i < 5; ++i) {
        QGraphicsTextItem *label = scene->addText(QString("%1\n%2").arg(artistData[i].first).arg(artistData[i].second));
        label->setPos(maxPoints[i] + QPointF(-40, -40)); // Подписи на внешних вершинах
    }
    // Подпись среднего значения на одной из вершин внутреннего пятиугольника
    QGraphicsTextItem *avgLabel = scene->addText(QString("Average: %1").arg(QString::number(averageRevenue, 'f', 2)));
    avgLabel->setPos(avgPoints[0] + QPointF(-30, -30)); // Размещаем подпись на первой вершине среднего пятиугольника

    // Отображаем сцену
    ui->graphicsView->setScene(scene);
    ui->graphicsView->show();
}

void MainWindow::showTop5ArtistsChart()
{
    QSqlQuery query(db);
    query.exec(R"(
        SELECT artists.Name AS ArtistName, ROUND(SUM(invoice_items.UnitPrice * invoice_items.Quantity), 2) AS Revenue
        FROM invoice_items
        JOIN tracks ON invoice_items.TrackId = tracks.TrackId
        JOIN albums ON tracks.AlbumId = albums.AlbumId
        JOIN artists ON albums.ArtistId = artists.ArtistId
        GROUP BY artists.ArtistId
        ORDER BY Revenue DESC
        LIMIT 5;
    )");

    // Создаём круговую диаграмму
    QPieSeries *series = new QPieSeries();

    while (query.next()) {
        QString artistName = query.value("ArtistName").toString();
        double revenue = query.value("Revenue").toDouble();

        series->append(artistName, revenue);
    }

    // Настройка сегментов для наглядности
    for (auto slice : series->slices()) {
        slice->setLabel(QString("%1: %2").arg(slice->label()).arg(slice->value()));
        slice->setLabelVisible(true);
    }

    // Создаём график
    QChart *chart = new QChart();
    chart->addSeries(series);
    chart->setTitle("Top 5 Artists by Revenue");
    chart->legend()->setAlignment(Qt::AlignBottom);

    // Отображаем диаграмму
    ui->chartView->setChart(chart);
    ui->chartView->setRenderHint(QPainter::Antialiasing);
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
