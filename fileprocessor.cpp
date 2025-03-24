#include "fileprocessor.h"
#include <QDebug>

FileProcessor::FileProcessor(QObject *parent) : QObject(parent) {}

void FileProcessor::processFiles(const QStringList &files, const QString &outputPath, bool deleteOriginals, quint64 xorValue, bool overwrite) {
    int totalFiles = files.size();
    int processedFiles = 0;

    // Обрабатываем файлы
    for (const QString &file : files) {
        QString inputFilePath = QFileInfo(file).absoluteFilePath(); // Полный путь к файлу
        QString outputFilePath = outputPath + "/" + QFileInfo(file).fileName(); // Путь для сохранения

        // Если файл уже существует, обрабатываем конфликт
        if (QFile::exists(outputFilePath) && !overwrite) {
            outputFilePath = getUniqueFileName(outputFilePath);
        }

        // Применяем XOR к файлу
        applyXor(inputFilePath, outputFilePath, xorValue);

        // Удаляем исходный файл, если нужно
        if (deleteOriginals) {
            QFile::remove(inputFilePath);
        }

        // Обновляем прогресс
        processedFiles++;
        int progress = (processedFiles * 100) / totalFiles;

        // Отправляем сигналы с прогрессом и статусом
        emit progressUpdated(progress);
        emit statusUpdated("Обработан файл: " + file);
    }

    emit finished(); // Сигнал о завершении обработки
}

void FileProcessor::applyXor(const QString &filePath, const QString &outputPath, quint64 xorValue) {
    QFile inputFile(filePath);
    QFile outputFile(outputPath);

    if (!inputFile.open(QIODevice::ReadOnly)) {
        qWarning() << "Не удалось открыть входной файл:" << filePath << "Причина:" << inputFile.errorString();
        return;
    }
    if (!outputFile.open(QIODevice::WriteOnly)) {
        qWarning() << "Не удалось открыть выходной файл:" << outputPath;
        return;
    }

    QDataStream in(&inputFile);
    QDataStream out(&outputFile);

    quint64 xorBuffer[1]; // Буфер для 8-байтного значения
    xorBuffer[0] = xorValue; // Записываем значение в буфер

    // Читаем файл побайтово и применяем XOR
    while (!in.atEnd()) {
        quint8 byte;
        in >> byte; // Читаем байт из файла

        // Применяем XOR к байту с соответствующим байтом из 8-байтного значения
        quint8 xorByte = reinterpret_cast<quint8*>(xorBuffer)[in.device()->pos() % 8];
        byte ^= xorByte;

        out << byte; // Записываем измененный байт в выходной файл
    }

    inputFile.close();
    outputFile.close();
}

QString FileProcessor::getUniqueFileName(const QString &filePath) {
    QFileInfo fileInfo(filePath);
    QString baseName = fileInfo.completeBaseName(); // Имя файла без расширения
    QString suffix = fileInfo.suffix(); // Расширение файла
    QString path = fileInfo.path(); // Путь к директории

    int counter = 1;
    QString newFilePath;
    do {
        newFilePath = QString("%1/%2_%3.%4").arg(path).arg(baseName).arg(counter).arg(suffix);
        counter++;
    } while (QFile::exists(newFilePath));

    return newFilePath;
}
