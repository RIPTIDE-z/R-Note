#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMessageBox>
#include <QJsonArray>
#include <QHeaderView>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , httpManager(new HttpManager(this))
{
    ui->setupUi(this);
    setupUI();

    connect(httpManager, &HttpManager::requestFinished,
        this, &MainWindow::onRequestFinished);
    connect(httpManager, &HttpManager::requestError,
        this, &MainWindow::onRequestError);

    // 启动时加载数据
    httpManager->getAllStudents();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::setupUI()
{
    // 设置表格
    ui->tableWidget->setColumnCount(4);
    ui->tableWidget->setHorizontalHeaderLabels({ "ID", "姓名", "年龄", "班级" });
    ui->tableWidget->horizontalHeader()->setStretchLastSection(true);
    ui->tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);

    // 设置列宽
    ui->tableWidget->setColumnWidth(0, 80);
    ui->tableWidget->setColumnWidth(1, 150);
    ui->tableWidget->setColumnWidth(2, 80);
}

void MainWindow::on_btnQuery_clicked()
{
    httpManager->getAllStudents();
}

void MainWindow::on_btnAdd_clicked()
{
    QString name = ui->lineEditName->text().trimmed();
    int age = ui->spinBoxAge->value();
    QString className = ui->lineEditClass->text().trimmed();

    if (name.isEmpty() || className.isEmpty()) {
        showMessage("姓名和班级不能为空！");
        return;
    }

    httpManager->addStudent(name, age, className);
}

void MainWindow::on_btnUpdate_clicked()
{
    QString idText = ui->lineEditId->text().trimmed();
    if (idText.isEmpty()) {
        showMessage("请先选择要更新的学生！");
        return;
    }

    int id = idText.toInt();
    QString name = ui->lineEditName->text().trimmed();
    int age = ui->spinBoxAge->value();
    QString className = ui->lineEditClass->text().trimmed();

    if (name.isEmpty() || className.isEmpty()) {
        showMessage("姓名和班级不能为空！");
        return;
    }

    httpManager->updateStudent(id, name, age, className);
}

void MainWindow::on_btnDelete_clicked()
{
    QString idText = ui->lineEditId->text().trimmed();
    if (idText.isEmpty()) {
        showMessage("请先选择要删除的学生！");
        return;
    }

    int ret = QMessageBox::question(this, "确认删除",
        "确定要删除该学生吗？",
        QMessageBox::Yes | QMessageBox::No);
    if (ret == QMessageBox::Yes) {
        httpManager->deleteStudent(idText.toInt());
    }
}

void MainWindow::on_tableWidget_cellClicked(int row, int column)
{
    Q_UNUSED(column);

    ui->lineEditId->setText(ui->tableWidget->item(row, 0)->text());
    ui->lineEditName->setText(ui->tableWidget->item(row, 1)->text());
    ui->spinBoxAge->setValue(ui->tableWidget->item(row, 2)->text().toInt());
    ui->lineEditClass->setText(ui->tableWidget->item(row, 3)->text());
}

void MainWindow::onRequestFinished(const QJsonObject& response)
{
    int code = response["code"].toInt();
    QString message = response["message"].toString();

    if (code == 200) {
        if (response.contains("data")) {
            QJsonValue dataValue = response["data"];

            if (dataValue.isArray()) {
                // 查询所有学生
                loadTableData(dataValue.toArray());
            }
            else {
                // 添加/更新/删除操作
                showMessage(message);
                clearInputFields();
                httpManager->getAllStudents(); // 刷新列表
            }
        }
        else {
            showMessage(message);
            httpManager->getAllStudents();
        }
    }
    else {
        showMessage("操纵失败: " + message);
    }
}

void MainWindow::onRequestError(const QString& error)
{
    showMessage("网络错误: " + error);
}

void MainWindow::loadTableData(const QJsonArray& students)
{
    ui->tableWidget->setRowCount(0);

    for (const QJsonValue& value : students) {
        QJsonObject student = value.toObject();

        int row = ui->tableWidget->rowCount();
        ui->tableWidget->insertRow(row);

        ui->tableWidget->setItem(row, 0, new QTableWidgetItem(
            QString::number(student["id"].toInt())));
        ui->tableWidget->setItem(row, 1, new QTableWidgetItem(
            student["name"].toString()));
        ui->tableWidget->setItem(row, 2, new QTableWidgetItem(
            QString::number(student["age"].toInt())));
        ui->tableWidget->setItem(row, 3, new QTableWidgetItem(
            student["className"].toString()));
    }
}

void MainWindow::clearInputFields()
{
    ui->lineEditId->clear();
    ui->lineEditName->clear();
    ui->spinBoxAge->setValue(18);
    ui->lineEditClass->clear();
}

void MainWindow::showMessage(const QString& message)
{
    ui->statusbar->showMessage(message, 3000);
    QMessageBox::information(this, "提示", message);
}