#ifndef FILEPROCESSOR_H
#define FILEPROCESSOR_H

#include <QObject>
#include <QFile>
#include <QDir>
#include <QMutex>
#include <QThread>


class FileProcessor : public QObject {
    Q_OBJECT

public:
    explicit FileProcessor(QObject *parent = nullptr);

public slots:
    void processFiles(const QStringList &files, const QString &outputPath, bool deleteOriginals, quint64 xorValue, bool overwrite);

signals:
    void progressUpdated(int value); // Сигнал для обновления прогресса
    void statusUpdated(const QString &status); // Сигнал для обновления статуса
    void finished(); // Сигнал о завершении обработки

private:
    void applyXor(const QString &filePath, const QString &outputPath, quint64 xorValue);
    QString getUniqueFileName(const QString &filePath);
    QMutex mutex; // Мьютекс для синхронизации
};

#endif // FILEPROCESSOR_H
