#include "mainwindow.h"
#include <QPainter>
#include <QMouseEvent>
#include <cmath>
#include <QTimer>
#include <QSound>
#include <QMessageBox>
#include <QMenu>
#include <QMenuBar>



//------------------------------------------全局变量--------------------------------------------//

#define CHESS_ONE_SOUND ":/res/sound/chessone.wav"//落子声音
//#define LOSE_SOUND ":/res/sound/lose.mp3"//QSound播放不了mp3格式的文件

//现代五子棋有15*15共255个棋位，因此画出横竖各14个格子，得到横竖个15条线，形成255个交叉
//点作为棋位
extern const int boardBlockNumber;//棋格的数目，横竖相同

const int boardBlockSize = 40; //棋格的大小，棋格为正方形，此处给出边长

const int boardMarkRadii = 3;//棋盘上5个标记的半径

const int boardMarginDistance = 30; // 棋盘边界距离窗口边界的距离

const int boardChessRadii = 15; // 棋子半径

const int boardChessOnMarkSize = 6; //根据鼠标位置判断落子的棋位，并在该处做一个
                                    //落子标记，落子标记为正方形，此处给出边长

const int boardChessPosRange = 20;  // 鼠标点击的模糊距离上限，当鼠标位置距离某个棋位的距离
                                    //小于该值时，就判定该棋位为用户想要落子的位置，并显示
                                    //落子标记，当鼠标在该位置点击时，在该棋位画出相应的棋子

const int delayAfterChessOneByHuman = 500;//人类玩家下完一个棋子后，要切换到AI玩家下。但是如果AI
                                        //下棋子下得太快的话，在界面上，人会感觉AI和人的棋子是
                                        //同时落下的，感觉不太好。所以在人下了一个棋子后，设置
                                        //一个延迟时间，然后再让AI下棋。单位为：毫秒。
//---------------------------------------------------------------------------------------//



//构造函数
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent)
{

    //----------------设置主窗口大小，其实就是设置棋盘大小--------------------------------------//

    setFixedSize(boardMarginDistance * 2 + boardBlockSize * boardBlockNumber, //宽度
                 boardMarginDistance * 2.5 + boardBlockSize * boardBlockNumber);//高度
    //-----------------------------------------------------------------------------------//


    //-------------------------------开启鼠标hover功能，即鼠标悬停事件-------------------------//
    setMouseTracking(true);
    //-----------------------------------------------------------------------------------//


    //--------------------------------------设置菜单---------------------------------------//
    QMenu *choice = menuBar()->addMenu(tr("选项"));
    QAction *playAgain = new QAction(tr("重玩"), this);
    connect(playAgain, SIGNAL(triggered()), this, SLOT(resetGame()));
    choice->addAction(playAgain);
    //-----------------------------------------------------------------------------------//

    //鼠标点击时的落子位置
    //初始化
    clickPosCol = -1;
    clickPosRow = -1;

    alphaMiao =  new GomokuAI;
}



//析构函数
MainWindow::~MainWindow()
{
    delete alphaMiao;
}



//-----------------------------------------重置游戏的函数函数---------------------------------------//
void MainWindow::resetGame()
{
    //棋盘的棋子清空
    for(int i = 0; i != boardBlockNumber + 1; ++i)
    {
        for(int j = 0; j != boardBlockNumber + 1; ++j)
        {
            alphaMiao->board[i][j] = 0;
        }
    }

    //重置后，人类玩家先手
    alphaMiao->playerFlag = true;
    alphaMiao->firstStepOfAI = true;
    alphaMiao->lastChessCol = -1;
    alphaMiao->lastChessRow = -1;
    alphaMiao->gameStatus = 0;
}

//----------------------------------------------------------------------------------------------//



//-------------------------------------------重绘事件处理函数-----------------------------------------//
void MainWindow::paintEvent(QPaintEvent *event)
{

    //********************************画出棋盘********************************//
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true); // 抗锯齿

    //横竖各boardBlockNumber个格子，需要横竖各画boardBlockNumber + 1条线
    for (int i = 0; i < boardBlockNumber + 1; i++)
    {
        //drawLine(int x1, int y1, int x2, int y2)
        //画出端点为(x1, x2)和(y1, y2)之间的线段
        //其中横坐标x列数，纵坐标y为行数
        //画出第i条竖线
        painter.drawLine(boardMarginDistance + boardBlockSize * i,
                         boardMarginDistance * 1.5,
                         boardMarginDistance + boardBlockSize * i,
                         size().height() - boardMarginDistance);

        //画出第i条横线
        painter.drawLine(boardMarginDistance,
                         boardMarginDistance * 1.5 + boardBlockSize * i,
                         size().width() - boardMarginDistance,
                         boardMarginDistance * 1.5 + boardBlockSize * i);
    }
    QBrush brush;
    brush.setStyle(Qt::SolidPattern);
    brush.setColor(Qt::black);
    painter.setBrush(brush);
    //画出5个标记点
    painter.drawEllipse(boardMarginDistance + 3 * boardBlockSize - boardMarkRadii,
                        boardMarginDistance * 1.5 + 3 * boardBlockSize - boardMarkRadii,
                        boardMarkRadii * 2,
                        boardMarkRadii * 2);

    painter.drawEllipse(boardMarginDistance + 11 * boardBlockSize - boardMarkRadii,
                        boardMarginDistance * 1.5 + 3 * boardBlockSize - boardMarkRadii,
                        boardMarkRadii * 2,
                        boardMarkRadii * 2);
    painter.drawEllipse(boardMarginDistance + 3 * boardBlockSize - boardMarkRadii,
                        boardMarginDistance * 1.5 + 11 * boardBlockSize - boardMarkRadii,
                        boardMarkRadii * 2,
                        boardMarkRadii * 2);
    painter.drawEllipse(boardMarginDistance + 11 * boardBlockSize - boardMarkRadii,
                        boardMarginDistance * 1.5 + 11 * boardBlockSize - boardMarkRadii,
                        boardMarkRadii * 2,
                        boardMarkRadii * 2);
    painter.drawEllipse(boardMarginDistance + 7 * boardBlockSize - boardMarkRadii,
                        boardMarginDistance * 1.5 + 7 * boardBlockSize - boardMarkRadii,
                        boardMarkRadii * 2,
                        boardMarkRadii * 2);

    //***********************************************************************//



    //*******************************绘制落子标记******************************//
    if (clickPosCol >=0 &&
            clickPosCol <= boardBlockNumber &&
            clickPosRow >=0 &&
            clickPosRow <= boardBlockNumber)
    {
        brush.setColor(Qt::black);
        painter.setBrush(brush);
        painter.drawRect(boardMarginDistance + boardBlockSize * clickPosCol - boardChessOnMarkSize * 0.5,
                         boardMarginDistance * 1.5 + boardBlockSize * clickPosRow - boardChessOnMarkSize * 0.5,
                         boardChessOnMarkSize,
                         boardChessOnMarkSize);
    }

    //***********************************************************************//



    //*******************************绘制棋子******************************//

    for(int i = 0; i != boardBlockNumber + 1; ++i)
    {
        for(int j = 0; j != boardBlockNumber + 1; ++j)
        {
            if(alphaMiao->board[i][j] == 1)
            {
                //该棋位是数字1，表示黑棋
                brush.setColor(Qt::black);
                //用brush画椭圆
                //做一个矩形的内接椭圆
                painter.setBrush(brush);
                painter.drawEllipse(boardMarginDistance + j * boardBlockSize - boardChessRadii,
                                    boardMarginDistance * 1.5 + i * boardBlockSize - boardChessRadii,
                                    boardChessRadii * 2,
                                    boardChessRadii * 2);
            }
            else if(alphaMiao->board[i][j] == -1)
            {
                //该棋位是数字-1，表示白棋
                brush.setColor(Qt::white);
                //用brush画椭圆
                //做一个矩形的内接椭圆
                painter.setBrush(brush);
                painter.drawEllipse(boardMarginDistance + j * boardBlockSize - boardChessRadii,
                                    boardMarginDistance * 1.5 + i * boardBlockSize - boardChessRadii,
                                    boardChessRadii * 2,
                                    boardChessRadii * 2);
            }
            else
            {
                //该棋位是0，没有棋子
            }
        }
    }
    //***********************************************************************//



    //*********************************判断游戏状态********************************//
    if(alphaMiao->gameStatus != 3)//当前状态不是正在评估游戏状态时，才调用游戏评估函数
    {
        QTimer::singleShot(10, this, SLOT(evaluateStatus()));
    }
    else
    {
        //do nothing
    }
    //***********************************************************************//
}
//----------------------------------------------------------------------------------------------//




//----------------------------------------------鼠标移动事件-----------------------------------------//
void MainWindow::mouseMoveEvent(QMouseEvent *event)
{
    if(alphaMiao->playerFlag)//若轮到人类玩家下棋时，才根据鼠标判断落子位置
    {
        //通过鼠标悬停事件hover确定落子标记
        //鼠标位置
        int x = event->x();
        int y = event->y();

        //当鼠标位于棋盘的某个格子内时，才判断落子的棋位
        if (x >= boardMarginDistance &&
                x <= size().width() - boardMarginDistance &&
                y >= boardMarginDistance * 1.5 &&
                y <= size().height() - boardMarginDistance)
        {
            //获取鼠标所在棋格左上角棋位在棋盘上的列数(0~14)和行数(0~14)
            int col = (x - boardMarginDistance)/boardBlockSize;//列数
            int row = (y - boardMarginDistance * 1.5)/boardBlockSize;//行数

            //计算鼠标所在棋格左上角棋位的坐标
            int leftTopPosX = boardMarginDistance + boardBlockSize * col;
            int leftTopPosY = boardMarginDistance *1.5 + boardBlockSize * row;


            //计算落子位置为该棋格的四个棋位中的哪一个
            //根据之前设置的模糊距离上限，落子位置只能是四个棋位中的一个，或者一个都不是
            int len = 0;//用来存储鼠标位置与四个棋位之间的欧氏距离

            //鼠标位置与左上角棋格之间的欧氏距离
            len = sqrt((x - leftTopPosX) * (x - leftTopPosX) +
                       (y - leftTopPosY) * (y - leftTopPosY));

            if (len < boardChessPosRange)
            {
                clickPosCol = col;
                clickPosRow = row;
            }
            else
            {
                //鼠标位置与右上角棋格之间的欧氏距离
                len = sqrt((x - leftTopPosX - boardBlockSize) * (x - leftTopPosX - boardBlockSize) +
                           (y - leftTopPosY) * (y - leftTopPosY));
                if(len < boardChessPosRange)
                {
                    clickPosCol = col + 1;
                    clickPosRow = row;
                }
                else
                {
                    //鼠标位置与左下角棋格之间的欧氏距离
                    len = sqrt((x - leftTopPosX) * (x - leftTopPosX) +
                               (y - leftTopPosY - boardBlockSize) * (y - leftTopPosY - boardBlockSize));
                    if (len < boardChessPosRange)
                    {
                        clickPosCol = col;
                        clickPosRow = row + 1;
                    }
                    else
                    {
                        //鼠标位置与右下角棋格之间的欧氏距离
                        len = sqrt((x - leftTopPosX - boardBlockSize) * (x - leftTopPosX - boardBlockSize) +
                                   (y - leftTopPosY - boardBlockSize) * (y - leftTopPosY - boardBlockSize));
                        if (len < boardChessPosRange)
                        {
                            clickPosCol = col + 1;
                            clickPosRow = row + 1;
                        }
                        else
                        {
                            clickPosCol = -1;
                            clickPosRow = -1;
                        }
                    }
                }
            }
        }
        //获取鼠标点击时的落子位置后，重新绘图
        update();
    }
    else//若此时轮到AI下棋
    {
        //do nothin
    }

}
//----------------------------------------------------------------------------------------------//


//------------------------------------------鼠标点击后释放事件--------------------------------------//
void MainWindow::mouseReleaseEvent(QMouseEvent *event)
{
    if(alphaMiao->playerFlag)
    {
        if(clickPosRow != -1 && clickPosCol != -1 && alphaMiao->board[clickPosRow][clickPosCol] == 0)
        {
            //鼠标点击位置的棋格没有棋子
            //更新该棋位的状态
            alphaMiao->updateBoard(clickPosRow, clickPosCol);//在更新棋盘状态时进行换手
            QSound::play(CHESS_ONE_SOUND);//播放落子声音
            //重画
            update();

            //此时人类玩家下了一步棋
            //可能此时分出了胜负，游戏被初始化
            //也可能此时没有分出胜负，需要AI下一个棋子

            if(alphaMiao->gameStatus == 0 && !alphaMiao->playerFlag)
            {
                QTimer::singleShot(delayAfterChessOneByHuman, this, SLOT(chessOneByAI()));
            }
            else
            {
                //do nothing
            }
        }
        else
        {
            //do nothing
        }
    }
    else
    {
        //do nothing
    }
}
//----------------------------------------------------------------------------------------------//







//------------------------------------------AI下一个棋子--------------------------------------//

void MainWindow::chessOneByAI()
{
    //调用AI下棋的函数，目前还没写
    alphaMiao->chessOneByAI();

    QSound::play(CHESS_ONE_SOUND);//播放落子声音
    //AI下完一个棋子后需要再次重绘
    update();
}

//----------------------------------------------------------------------------------------------//


//------------------------------判断当前局势：AI胜利、人类胜利、平局-------------------------------------//
void MainWindow::evaluateStatus()
{
    if(alphaMiao->gameStatus != 0)
    {
        QString str;
        if(alphaMiao->gameStatus == -1)//AI玩家胜利
        {
            str = " AI玩家获胜!";
        }
        else if(alphaMiao->gameStatus == 1)//人类玩家胜利
        {
            str = "人类玩家获胜!";
        }
        else//alphaMiao->gameStatus == 2, 平局
        {
            str = "平局!       ";
        }

        alphaMiao->gameStatus = 3;//标记当前状态为：正在评估游戏的状态
        QMessageBox::StandardButton btnValue = QMessageBox::information(this, "gameover", str);

        if(btnValue == QMessageBox::Ok)
        {
            resetGame();
        }
    }
    else
    {
        //do nothing
    }
}
//----------------------------------------------------------------------------------------------//
