#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTableWidget>
#include <QLineEdit>
#include <QSpinBox>
#include <QPushButton>
#include <QJsonObject>
#include "httpmanager.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

private slots:
    void on_btnQuery_clicked();
    void on_btnAdd_clicked();
    void on_btnUpdate_clicked();
    void on_btnDelete_clicked();
    void on_tableWidget_cellClicked(int row, int column);

    void onRequestFinished(const QJsonObject& response);
    void onRequestError(const QString& error);

private:
    Ui::MainWindow* ui;
    HttpManager* httpManager;

    void setupUI();
    void loadTableData(const QJsonArray& students);
    void clearInputFields();
    void showMessage(const QString& message);
};

#endif // MAINWINDOW_H