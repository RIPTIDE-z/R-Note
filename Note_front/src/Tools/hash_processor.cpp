#include "hash_processor.h"

QString HashEncrypt(const QString& code) 
{
    QByteArray codeBytes = code.toUtf8();
    QByteArray hashBytes = QCryptographicHash::hash(
        codeBytes,
        QCryptographicHash::Sha256
    );

    return hashBytes.toHex();
}