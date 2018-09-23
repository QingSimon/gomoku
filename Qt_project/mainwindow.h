#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "gomokuai.h"


class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

protected:

    //重绘事件处理函数
    //每次需要重绘窗口时，画出棋盘和棋盘上的棋子
    void paintEvent(QPaintEvent *event);

    //追踪鼠标位置
    void mouseMoveEvent(QMouseEvent *event);

    void mouseReleaseEvent(QMouseEvent *event);


private:

    //鼠标点击时的落子位置
    int clickPosCol, clickPosRow;
    GomokuAI  *alphaMiao;



private slots:
    void chessOneByAI();//AI下一步棋
    void evaluateStatus();//判断当前局势：活局、胜负已分、平局

    //重置游戏的函数函数
    void resetGame();
};

#endif // MAINWINDOW_H
