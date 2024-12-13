
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
    void showRevenueByGenre();
    void showTop3ArtistsByGenre();
    void showTop5ArtistsOverall();
    void showInteractiveMap();


private:
    Ui::MainWindow *ui;
    QSqlDatabase db;
    void displayTable(QSqlQuery query, const QStringList &headers);
    void displayMap(QMap<QString, QMap<QString, double>> &data);
};

#endif // MAINWINDOW_H
