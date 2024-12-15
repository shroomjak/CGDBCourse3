#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlTableModel>
#include <QTableView>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QMap>
#include <QtCharts>

QT_CHARTS_USE_NAMESPACE

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void showMonthlySales();
    void showMonthlySalesChart();

    void showRevenueByGenre();
    void showRevenueByGenreChart();

    void showTop3ArtistsByGenre();
    void showTop3ArtistsByGenreChart();

    void showTop5ArtistsOverall();
    void showTop5ArtistsPentagonChart();
    void showTop5ArtistsChart();

    void showInteractiveMap();
    void clearScene();


private:
    Ui::MainWindow *ui;
    QSqlDatabase db;
    void displayTable(QSqlQuery query, const QStringList &headers);
    void displayMap(QMap<QString, QMap<QString, double>> &data);
    QMap<QString, QColor> GenerateGenreColors(const QMap<QString, QMap<QString, double>> &mapData);
    void AddLegendToScene(QGraphicsScene *scene, const QMap<QString, QColor> &genreColors);
};

#endif // MAINWINDOW_H
