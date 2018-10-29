#include "JSONGUIPassWidget.h"
#include "ui_JSONGUIPassWidget.h"

#include <QDebug>
#include "JGMTop.h"
#include "JSONScrollWidget.h"
#include "QLabelDrag.h"




JSONGUIPassWidget::JSONGUIPassWidget(const JGMPassRef & inRef, QWidget *parent) :
	QWidget(parent),
	_pass(inRef),
	ui(new Ui::JSONGUIPassWidget)
{
	ui->setupUi(this);
	
	setAcceptDrops(true);
	
	if (_pass != nullptr)	{
		prepareUIItems();
		refreshUIItems();
	}
}

JSONGUIPassWidget::~JSONGUIPassWidget()
{
	delete ui;
}




void JSONGUIPassWidget::prepareUIItems()	{
	//qDebug() << __PRETTY_FUNCTION__;
	
	prepareDeleteLabel( (ui->deleteLabel) );
	prepareBufferNameEdit( (ui->targetBufferEdit) );
	preparePBufferCBox( (ui->persistentCBox) );
	prepareFBufferCBox( (ui->floatCBox) );
	prepareCustWidthEdit( (ui->widthEdit) );
	prepareCustHeightEdit( (ui->heightEdit) );
}
void JSONGUIPassWidget::refreshUIItems()	{
	//qDebug() << __PRETTY_FUNCTION__;
	
	refreshPassTitleLabel( (ui->titleLabel) );
	refreshBufferNameEdit( (ui->targetBufferEdit) );
	refreshPBufferCBox( (ui->persistentCBox) );
	refreshFBufferCBox( (ui->floatCBox) );
	refreshCustWidthEdit( (ui->widthEdit) );
	refreshCustHeightEdit( (ui->heightEdit) );
}




void JSONGUIPassWidget::dragEnterEvent(QDragEnterEvent * e)	{
	//qDebug() << __PRETTY_FUNCTION__;
	const QMimeData		*md = e->mimeData();
	if (md!=nullptr && md->hasFormat("text/JGMPassDrag"))	{
		QPoint		tmpPoint = e->pos();
		QSize		tmpSize = frameSize();
		if (tmpPoint.y() > tmpSize.height()/2)
			_dropEdge = Qt::BottomEdge;
		else
			_dropEdge = Qt::TopEdge;
		update();
		e->acceptProposedAction();
	}
}
void JSONGUIPassWidget::dragMoveEvent(QDragMoveEvent * e)	{
	//qDebug() << __PRETTY_FUNCTION__;
	QPoint		tmpPoint = e->pos();
	QSize		tmpSize = frameSize();
	if (tmpPoint.y() > tmpSize.height()/2)
		_dropEdge = Qt::BottomEdge;
	else
		_dropEdge = Qt::TopEdge;
	update();
	e->accept();
}
void JSONGUIPassWidget::dragLeaveEvent(QDragLeaveEvent * e)	{
	//qDebug() << __PRETTY_FUNCTION__;
	_dropEdge = Qt::LeftEdge;
	update();
	e->accept();
}
void JSONGUIPassWidget::dropEvent(QDropEvent * e)	{
	qDebug() << __PRETTY_FUNCTION__;
	
	const QMimeData		*md = e->mimeData();
	QVariant			mdvar = QVariant(md->data("text/JGMPassDrag"));
	int					srcIndex = mdvar.toInt();
	
	JGMTop			*top = _pass->top();
	int				dstIndex = top->indexOfPass(*_pass);
	if (srcIndex == dstIndex)
		return;
	if (_dropEdge == Qt::BottomEdge)
		++dstIndex;
	if (dstIndex > srcIndex)
		--dstIndex;

	qDebug() << "accepting drop from " << srcIndex << ", dstIndex is " << dstIndex;
	JGMCPassArray		&passesC = top->passesContainer();
	QVector<JGMPassRef>		&passesV = passesC.contents();
	passesV.move(srcIndex, dstIndex);
	
	_dropEdge = Qt::LeftEdge;
	update();
	e->accept();
	
	QTimer::singleShot(50, []()	{
		RecreateJSONAndExport();
	});
}




void JSONGUIPassWidget::prepareDragLabel(QLabelDrag * dragLabel)	{
	JGMTop		*top = _pass->top();
	if (top != nullptr)	{
		//	configure the drag label so it's "text/JGMPassDrag" and its data is the index of my pass
		ui->dragLabel->mimeType = QString("text/JGMPassDrag");
		//ui->dragLabel->dragVariant = QVariant( QString("frisky dingo") );
		ui->dragLabel->dragVariant = QVariant( top->indexOfPass(*_pass) );
		
	}
}
void JSONGUIPassWidget::prepareDeleteLabel(QLabelClickable * deleteLabel)	{
	QObject::disconnect(deleteLabel, 0, 0, 0);
	
	QObject::connect(deleteLabel, &QLabelClickable::clicked, [&]()	{
		if (_pass == nullptr)
			return;
		JGMTop		*top = _pass->top();
		if (top == nullptr)
			return;
		if (top->deletePass(_pass))	{
			RecreateJSONAndExport();
		}
	});
}
void JSONGUIPassWidget::prepareBufferNameEdit(QLineEdit * bufferNameEdit)	{
	QObject::disconnect(bufferNameEdit, 0, 0, 0);
	
	QObject::connect(bufferNameEdit, &QLineEdit::editingFinished, [&]()	{
		//	if the name hasn't changed, bail
		QJsonValue		origNameValue = _pass->value("TARGET");
		QString			origName = (origNameValue.isString()) ? origNameValue.toString() : QString("");
		QString			proposedName = bufferNameEdit->text();
		if (proposedName == origName)
			return;
		//	if there is already a buffer with that name, restore the original name and bail
		JGMTop			*top = _pass->top();
		QVector<JGMPassRef>		matchingPasses = top->getPassesRenderingToBufferNamed(proposedName);
		if (matchingPasses.size() > 0)	{
			bufferNameEdit->setText(origName);
			return;
		}
		//	update the attribute
		if (proposedName.size() < 1)
			_pass->setValue("TARGET", QJsonValue::Undefined);
		else
			_pass->setValue("TARGET", QJsonValue(proposedName));
		//	deselect the text, remove focus from the widget
		bufferNameEdit->deselect();
		bufferNameEdit->clearFocus();
		//	recreate the json & export the file
		RecreateJSONAndExport();
	});
}
void JSONGUIPassWidget::preparePBufferCBox(QCheckBox * pbufferCBox)	{
	QObject::disconnect(pbufferCBox, 0, 0, 0);
	
	QObject::connect(pbufferCBox, &QCheckBox::clicked, [&]()	{
		//	if it's checked, add it
		if (pbufferCBox->isChecked())	{
			_pass->setValue("PERSISTENT", QJsonValue(true));
		}
		//	else it's not checked- just remove the value entirely
		else	{
			_pass->setValue("PERSISTENT", QJsonValue::Undefined);
		}
		//	recreate the JSON & export the file
		RecreateJSONAndExport();
	});
}
void JSONGUIPassWidget::prepareFBufferCBox(QCheckBox * fbufferCBox)	{
	QObject::disconnect(fbufferCBox, 0, 0, 0);
	
	QObject::connect(fbufferCBox, &QCheckBox::clicked, [&]()	{
		//	if it's checked, add it
		if (fbufferCBox->isChecked())	{
			_pass->setValue("FLOAT", QJsonValue(true));
		}
		//	else it's not checked- just remove the value entirely
		else	{
			_pass->setValue("FLOAT", QJsonValue::Undefined);
		}
		//	recreate the JSON & export the file
		RecreateJSONAndExport();
	});
}
void JSONGUIPassWidget::prepareCustWidthEdit(QLineEdit * custWidthEdit)	{
	QObject::disconnect(custWidthEdit, 0, 0, 0);
	
	QObject::connect(custWidthEdit, &QLineEdit::editingFinished, [&]()	{
		//	if the width string hasn't changed, bail
		QJsonValue		origWidthVal = _pass->value("WIDTH");
		QString			origWidth = (origWidthVal.isUndefined()) ? QString("") : origWidthVal.toString();
		QString			proposedWidth = custWidthEdit->text();
		if (origWidth == proposedWidth)
			return;
		//	update the value
		if (proposedWidth.size() < 1)
			_pass->setValue("WIDTH", QJsonValue::Undefined);
		else
			_pass->setValue("WIDTH", proposedWidth);
		//	deselect the text, remove focus from the widget
		custWidthEdit->deselect();
		custWidthEdit->clearFocus();
		//	recreate the json & export the file
		RecreateJSONAndExport();
	});
}
void JSONGUIPassWidget::prepareCustHeightEdit(QLineEdit * custHeightEdit)	{
	QObject::disconnect(custHeightEdit, 0, 0, 0);
	
	QObject::connect(custHeightEdit, &QLineEdit::editingFinished, [&]()	{
		//	if the height string hasn't changed, bail
		QJsonValue		origHeightVal = _pass->value("HEIGHT");
		QString			origHeight = (origHeightVal.isUndefined()) ? QString("") : origHeightVal.toString();
		QString			proposedHeight = custHeightEdit->text();
		if (origHeight == proposedHeight)
			return;
		//	update the value
		if (proposedHeight.size() < 1)
			_pass->setValue("HEIGHT", QJsonValue::Undefined);
		else
			_pass->setValue("HEIGHT", proposedHeight);
		//	deselect the text, remove focus from the widget
		custHeightEdit->deselect();
		custHeightEdit->clearFocus();
		//	recreate the json & export the file
		RecreateJSONAndExport();
	});
}


void JSONGUIPassWidget::refreshPassTitleLabel(QLabel * passNameLabel)	{
	JGMTop			*top = _pass->top();
	int				passIndex = top->indexOfPass(*_pass);
	passNameLabel->setText( QString("PASSINDEX %1").arg(passIndex) );
}
void JSONGUIPassWidget::refreshBufferNameEdit(QLineEdit * bufferNameEdit)	{
	QString			tmpString = _pass->value("TARGET").toString();
	bufferNameEdit->setText(tmpString);
}
void JSONGUIPassWidget::refreshPBufferCBox(QCheckBox * pbufferCBox)	{
	QJsonValue		targetNameVal = _pass->value("TARGET");
	QString			targetName;
	if (targetNameVal.isString())
		targetName = targetNameVal.toString();
	else
		targetName = QString("");
	
	JGMTop			*top = _pass->top();
	JGMPassRef		pbuffer = (targetName.size()<1) ? nullptr : top->getPersistentPassNamed(targetName);
	pbufferCBox->setCheckState( (pbuffer==nullptr) ? Qt::Unchecked : Qt::Checked );
}
void JSONGUIPassWidget::refreshFBufferCBox(QCheckBox * fbufferCBox)	{
	//qDebug() << __PRETTY_FUNCTION__;
	QJsonValue			tmpVal = _pass->value("FLOAT");
	fbufferCBox->setCheckState(((tmpVal.isBool()&&tmpVal.toBool()) || (tmpVal.isDouble()&&tmpVal.toDouble()>0.0)) ? Qt::Checked : Qt::Unchecked);
	//int				tmpInt = _pass->value("FLOAT").toInt();
	//qDebug() << "\ttmpInt is " << tmpInt;
	//fbufferCBox->setCheckState( (tmpInt>0) ? Qt::Checked : Qt::Unchecked );
}
void JSONGUIPassWidget::refreshCustWidthEdit(QLineEdit * custWidthEdit)	{
	QJsonValue		tmpVal = _pass->value("WIDTH");
	QString			tmpString;
	if (!tmpVal.isString())
		tmpString = QString("");
	else
		tmpString = tmpVal.toString();
	custWidthEdit->setText(tmpString);
}
void JSONGUIPassWidget::refreshCustHeightEdit(QLineEdit * custHeightEdit)	{
	QJsonValue		tmpVal = _pass->value("HEIGHT");
	QString			tmpString;
	if (!tmpVal.isString())
		tmpString = QString("");
	else
		tmpString = tmpVal.toString();
	custHeightEdit->setText(tmpString);
}




void JSONGUIPassWidget::paintEvent(QPaintEvent * e)	{
	//qDebug() << __PRETTY_FUNCTION__;
	
	QRect			bounds(QPoint(), frameSize());
	QRect			drawRect;
	switch (_dropEdge)	{
	case Qt::TopEdge:
		drawRect = QRect(
			bounds.topLeft().x(),
			bounds.topLeft().y(),
			bounds.size().width(),
			13.0);
		break;
	case Qt::BottomEdge:
		drawRect = QRect(
			bounds.bottomLeft().x(),
			bounds.bottomLeft().y() - 13.0,
			bounds.size().width(),
			13.0);
		break;
	default:
		return;
	}
	
	
	QPainter		painter(this);
	painter.fillRect(drawRect, QBrush(QColor(255,0,0,255)));
}
