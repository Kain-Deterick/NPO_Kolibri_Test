#include "mainwindow.h"
#include "./ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    // Подключение кнопки "Старт"
    connect(ui->StartButton, &QPushButton::clicked, this, &MainWindow::onStartButtonClicked);
    // Подключение таймера
    connect(timer, &QTimer::timeout, this, &MainWindow::onTimerTimeout);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::onStartButtonClicked() {
    // Получаем параметры из интерфейса
    QString inputMask = ui->inputMaskEdit->text(); // Маска файлов
    QString outputPath = ui->outputPathEdit->text(); // Путь для сохранения
    bool deleteOriginals = ui->deleteSwitch->isChecked(); // Удалять исходные файлы
    quint64 xorValue = ui->xorValueEdit->text().toULongLong(nullptr, 16); // 8-байтное значение (HEX)
    bool overwrite = (ui->fileConflictComboBox->currentIndex() == 0); // Перезапись или переименование
    bool useTimer = (ui->modeComboBox->currentIndex() == 1); // Работа по таймеру

    // Проверка ввода
    if (inputMask.isEmpty() || outputPath.isEmpty() || ui->xorValueEdit->text().isEmpty()) {
        qWarning() << "Пожалуйста, заполните все поля!";
        return;
    }

    // Остановка таймера, если он был запущен
    if (timer->isActive()) {
        timer->stop();
    }

    // Получаем путь к папке и имя файла из маски
    QFileInfo maskInfo(inputMask);
    // Получаем список файлов
    QDir directory(maskInfo.absolutePath());

    // Получаем список файлов с полными путями
    QStringList files;
    const QStringList nameFilters = QStringList() << maskInfo.fileName();
    const QFileInfoList fileInfoList = directory.entryInfoList(nameFilters, QDir::Files);

    // Заполняем список полными путями к файлам
    for (const QFileInfo& fileInfo : fileInfoList) {
        files.append(fileInfo.absoluteFilePath());
    }

    // Разделяем файлы на четные и нечетные
    QStringList oddFiles, evenFiles;
    for (int i = 0; i < files.size(); ++i) {
        if (i % 2 == 0) {
            evenFiles.append(files[i]); // Четные файлы
        } else {
            oddFiles.append(files[i]); // Нечетные файлы
        }
    }

    // Если выбран режим таймера, запускаем его
    if (useTimer) {
        int interval = ui->intervalSpinBox->value() * 1000; // Интервал в миллисекундах
        timer->start(interval); // Запуск таймера
    } else {
        // Разовый запуск
        if (timer->isActive()) {
            timer->stop();
        }

        // Создаем два потока
        QThread *thread1 = new QThread;
        QThread *thread2 = new QThread;

        // Создаем два обработчика файлов
        FileProcessor *processor1 = new FileProcessor();
        FileProcessor *processor2 = new FileProcessor();

        // Перемещаем обработчики в потоки
        processor1->moveToThread(thread1);
        processor2->moveToThread(thread2);

        // Подключение сигналов и слотов для первого потока (четные файлы)
        connect(thread1, &QThread::started, processor1, [processor1, evenFiles, outputPath, deleteOriginals, xorValue, overwrite]() {
            processor1->processFiles(evenFiles, outputPath, deleteOriginals, xorValue, overwrite);
        });
        connect(processor1, &FileProcessor::progressUpdated, this, &MainWindow::updateProgress);
        connect(processor1, &FileProcessor::statusUpdated, this, &MainWindow::updateStatus);
        connect(processor1, &FileProcessor::finished, thread1, &QThread::quit);
        connect(processor1, &FileProcessor::finished, processor1, &FileProcessor::deleteLater);
        connect(thread1, &QThread::finished, thread1, &QThread::deleteLater);

        // Подключение сигналов и слотов для второго потока (нечетные файлы)
        connect(thread2, &QThread::started, processor2, [processor2, oddFiles, outputPath, deleteOriginals, xorValue, overwrite]() {
            processor2->processFiles(oddFiles, outputPath, deleteOriginals, xorValue, overwrite);
        });
        connect(processor2, &FileProcessor::progressUpdated, this, &MainWindow::updateProgress);
        connect(processor2, &FileProcessor::statusUpdated, this, &MainWindow::updateStatus);
        connect(processor2, &FileProcessor::finished, thread2, &QThread::quit);
        connect(processor2, &FileProcessor::finished, processor2, &FileProcessor::deleteLater);
        connect(thread2, &QThread::finished, thread2, &QThread::deleteLater);

        // Запуск потоков
        thread1->start();
        thread2->start();
    }
}

void MainWindow::onTimerTimeout() {
    // Получаем параметры из интерфейса
    QString inputMask = ui->inputMaskEdit->text(); // Маска файлов
    QString outputPath = ui->outputPathEdit->text(); // Путь для сохранения
    bool deleteOriginals = ui->deleteSwitch->isChecked(); // Удалять исходные файлы
    quint64 xorValue = ui->xorValueEdit->text().toULongLong(nullptr, 16); // 8-байтное значение (HEX)
    bool overwrite = (ui->fileConflictComboBox->currentIndex() == 0); // Перезапись или переименование

    // Получаем путь к папке и имя файла из маски
    QFileInfo maskInfo(inputMask);
    // Получаем список файлов
    QDir directory(maskInfo.absolutePath());

    QStringList files;
    const QStringList nameFilters = QStringList() << maskInfo.fileName();
    const QFileInfoList fileInfoList = directory.entryInfoList(nameFilters, QDir::Files);

    // Заполняем список полными путями к файлам
    for (const QFileInfo& fileInfo : fileInfoList) {
        files.append(fileInfo.absoluteFilePath());
    }

    // Разделяем файлы на четные и нечетные
    QStringList oddFiles, evenFiles;
    for (int i = 0; i < files.size(); ++i) {
        if (i % 2 == 0) {
            evenFiles.append(files[i]); // Четные файлы
        } else {
            oddFiles.append(files[i]); // Нечетные файлы
        }
    }

    // Создаем два потока
    QThread *thread1 = new QThread;
    QThread *thread2 = new QThread;

    // Создаем два обработчика файлов
    FileProcessor *processor1 = new FileProcessor();
    FileProcessor *processor2 = new FileProcessor();

    // Перемещаем обработчики в потоки
    processor1->moveToThread(thread1);
    processor2->moveToThread(thread2);

    // Подключение сигналов и слотов для первого потока (четные файлы)
    connect(thread1, &QThread::started, processor1, [processor1, evenFiles, outputPath, deleteOriginals, xorValue, overwrite]() {
        processor1->processFiles(evenFiles, outputPath, deleteOriginals, xorValue, overwrite);
    });
    connect(processor1, &FileProcessor::progressUpdated, this, &MainWindow::updateProgress);
    connect(processor1, &FileProcessor::statusUpdated, this, &MainWindow::updateStatus);
    connect(processor1, &FileProcessor::finished, thread1, &QThread::quit);
    connect(processor1, &FileProcessor::finished, processor1, &FileProcessor::deleteLater);
    connect(thread1, &QThread::finished, thread1, &QThread::deleteLater);

    // Подключение сигналов и слотов для второго потока (нечетные файлы)
    connect(thread2, &QThread::started, processor2, [processor2, oddFiles, outputPath, deleteOriginals, xorValue, overwrite]() {
        processor2->processFiles(oddFiles, outputPath, deleteOriginals, xorValue, overwrite);
    });
    connect(processor2, &FileProcessor::progressUpdated, this, &MainWindow::updateProgress);
    connect(processor2, &FileProcessor::statusUpdated, this, &MainWindow::updateStatus);
    connect(processor2, &FileProcessor::finished, thread2, &QThread::quit);
    connect(processor2, &FileProcessor::finished, processor2, &FileProcessor::deleteLater);
    connect(thread2, &QThread::finished, thread2, &QThread::deleteLater);

    // Запуск потоков
    thread1->start();
    thread2->start();
}

void MainWindow::updateProgress(int value) {
    ui->progressBar->setValue(value); // Обновляем прогрессбар
}

void MainWindow::updateStatus(const QString &status) {
    ui->statusLabel->setText(status); // Обновляем статус
}
