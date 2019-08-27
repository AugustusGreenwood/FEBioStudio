#ifdef HAS_QUAZIP
#include <quazip5/JlCompress.h>
#include <quazip5/quazip.h>
#include <quazip5/quazipfile.h>

static void recurseAddDir(QDir d, QStringList & list) {

    QStringList qsl = d.entryList(QDir::NoDotAndDotDot | QDir::Dirs | QDir::Files);

    for (QString file : qsl) {

        QFileInfo finfo(QString("%1/%2").arg(d.path()).arg(file));

        if (finfo.isDir()) {

            QString dirname = finfo.fileName();
            QDir sd(finfo.filePath());

            recurseAddDir(sd, list);

        } else
        	list << QDir::toNativeSeparators(finfo.filePath());

    }
}

static bool archive(const QString & filePath, const QDir & dir, const QString & comment = QString("")) {

	QuaZip zip(filePath);
	zip.setFileNameCodec("IBM866");

	if (!zip.open(QuaZip::mdCreate)) {
		return false;
	}

	if (!dir.exists()) {
		return false;
	}

	QFile inFile;

	QStringList sl;
	recurseAddDir(dir, sl);

	QFileInfoList files;
	foreach (QString fn, sl) files << QFileInfo(fn);

	QuaZipFile outFile(&zip);

	char c;
	foreach(QFileInfo fileInfo, files) {

		if (!fileInfo.isFile())
			continue;

		QString fileNameWithRelativePath = fileInfo.filePath().remove(0, dir.absolutePath().length() + 1);

		inFile.setFileName(fileInfo.filePath());

		if (!inFile.open(QIODevice::ReadOnly)) {
			return false;
		}

		if(fileInfo.isSymLink())
		{
			if (!outFile.open(QIODevice::WriteOnly, QuaZipNewInfo(fileNameWithRelativePath, fileInfo.symLinkTarget()))) {
				return false;
			}
		}
		else
		{
			if (!outFile.open(QIODevice::WriteOnly, QuaZipNewInfo(fileNameWithRelativePath, fileInfo.filePath()))) {
				return false;
			}
		}

		while (inFile.getChar(&c) && outFile.putChar(c));

		if (outFile.getZipError() != UNZ_OK) {
			return false;
		}

		outFile.close();

		if (outFile.getZipError() != UNZ_OK) {
			return false;
		}

		inFile.close();
	}

	zip.close();

	if (zip.getZipError() != 0) {
		return false;
	}

	return true;
}

#endif
