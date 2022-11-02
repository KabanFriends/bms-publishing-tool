#include "dialogs/P2MapPublisher.h"

#include "P2DialogConfig.h"

#include <KeyValue.h>
#include <cmath>
#include <regex>

using namespace ui;

CP2MapPublisher::CP2MapPublisher( QWidget *pParent ) :
	CP2MapPublisher( pParent, false )
{
}

CP2MapPublisher::CP2MapPublisher( QWidget *pParent, bool edit ) :
	QDialog( pParent )
{
	m_EditItemIndex = -1;
	m_editItemDetails = SteamUGCDetails_t();

	this->setWindowTitle( tr( "Publish File" ) );
	m_edit = edit;
	auto pDialogLayout = new QGridLayout( this );
	auto pFindLabel = new QLabel( tr( "Preview Image:" ), this );

	QPixmap tempMap = QPixmap( ":/zoo_textures/SampleImage.png" );
	tempMap = tempMap.scaled( 240, 240, Qt::IgnoreAspectRatio );
	pImageLabel = new QLabel( this );
	pImageLabel->setPixmap( tempMap );
	pImageLabel->setMaximumSize( 240, 240 );
	m_advancedOptionsWindow = new QDialog( this );
	AO = new CP2PublisherAdvancedOptions();
	AO->setupUi( m_advancedOptionsWindow );
	AO->treeWidget->setSortingEnabled( false );

	auto pBrowseButton = new QPushButton( this );
	pBrowseButton->setText( tr( "Browse..." ) );
	pBrowseButton->setFixedSize( 478 / 2, 20 );

	pAdvancedOptionsButton = new QPushButton( this );
	pAdvancedOptionsButton->setText( tr( "Advanced Options." ) );
	pAdvancedOptionsButton->setFixedSize( 478 / 2, 40 );
	AO->disableTagWidget( true );

	pSteamToSAgreement = new QCheckBox( tr( "I accept the terms of the Steam Workshop Contribution Agreement." ), this );

	auto pAgreementButton = new QPushButton( this );
	pAgreementButton->setText( tr( "View Agreement" ) );
	pAgreementButton->setFixedSize( 478 / 2, 20 );

	auto pButtonBox = new QDialogButtonBox( Qt::Orientation::Horizontal, this );
	auto pOKButton = pButtonBox->addButton( tr( m_edit ? "Update" : "Upload" ), QDialogButtonBox::ApplyRole );
	auto pCloseButton = pButtonBox->addButton( tr( "Cancel" ), QDialogButtonBox::RejectRole );

	auto pTitleDescLayout = new QVBoxLayout( this );
	pTitleDescLayout->setSpacing( 10 );
	auto pTitle = new QLabel( tr( "Title:" ), this );
	pTitleLine = new QLineEdit( this );
	auto pDesc = new QLabel( tr( "Description:" ), this );
	pDescLine = new QTextEdit( this );
	pDescLine->setMinimumHeight( 200 );
	pDescLine->setMaximumHeight( 200 );
	auto pFile = new QLabel( tr( "Content Folder:" ), this );

	auto pFileLayout = new QHBoxLayout( this );
	pFileEntry = new QLineEdit( this );
	pFileEntry->setReadOnly( true );
	auto pBrowseButton2 = new QPushButton( tr( "Browse..." ), this );
	pFileLayout->addWidget( pFileEntry );
	pFileLayout->addWidget( pBrowseButton2 );

	pTitleDescLayout->addWidget( pTitle, 0, Qt::AlignTop );
	pTitleDescLayout->addWidget( pTitleLine, 0, Qt::AlignTop );
	pTitleDescLayout->addWidget( pDesc, 0, Qt::AlignTop );
	pTitleDescLayout->addWidget( pDescLine, 0, Qt::AlignTop );
	pTitleDescLayout->addWidget( pFile, 0, Qt::AlignTop );
	pTitleDescLayout->addLayout( pFileLayout, 0 );

	pDialogLayout->setHorizontalSpacing( 25 );

	pDialogLayout->addWidget( pFindLabel, 0, 0 );
	pDialogLayout->addWidget( pImageLabel, 1, 0 );
	pDialogLayout->addWidget( pBrowseButton, 2, 0 );
	pDialogLayout->addWidget( pAdvancedOptionsButton, 3, 0 );
	pDialogLayout->addWidget( pSteamToSAgreement, 9, 0, 1, 2, Qt::AlignBottom );
	pDialogLayout->addWidget( pAgreementButton, 10, 0, Qt::AlignBottom );
	pDialogLayout->addWidget( pButtonBox, 12, 0, 1, 2, Qt::AlignLeft );
	pDialogLayout->addLayout( pTitleDescLayout, 0, 1, 8, 8, Qt::AlignTop );

	connect( pBrowseButton, &QPushButton::pressed, this, &CP2MapPublisher::OpenImageFileExplorer );
	connect( pBrowseButton2, &QPushButton::pressed, this, &CP2MapPublisher::OpenBSPFileExplorer );
	connect( pOKButton, &QPushButton::pressed, this, &CP2MapPublisher::onOKPressed );
	connect( pCloseButton, &QPushButton::pressed, this, &CP2MapPublisher::onClosePressed );
	connect( pAdvancedOptionsButton, &QPushButton::pressed, this, &CP2MapPublisher::onAdvancedClicked );
	connect( pAgreementButton, &QPushButton::pressed, this, &CP2MapPublisher::onAgreementButtonPressed );

	this->setLayout( pDialogLayout );
	this->setFixedSize( this->sizeHint() );
	this->setWindowFlag( Qt::WindowContextHelpButtonHint, false );
}

void CP2MapPublisher::onAdvancedClicked()
{
	AO->label_5->setEnabled( m_edit );
	AO->textEdit->setEnabled( m_edit );
	m_advancedOptionsWindow->exec();
}

void CP2MapPublisher::LoadCreatingItem()
{
	SteamAPICall_t hApiCreateItemHandle = SteamUGC()->CreateItem( CP2MapMainMenu::ConsumerID, k_EWorkshopFileTypeCommunity );
	m_CallResultCreateItem.Set( hApiCreateItemHandle, this, &CP2MapPublisher::OnCreateItem );
	SteamHelper::StartLoopCall();
}

void CP2MapPublisher::UpdateItem( PublishedFileId_t itemID )
{
	UGCUpdateHandle_t hUpdateHandle = SteamUGC()->StartItemUpdate( CP2MapMainMenu::ConsumerID, itemID );

	SteamUGC()->SetItemContent( hUpdateHandle, defaultFileLocBSP.toStdString().c_str() );

	if ( !m_edit || ( m_edit && QString( m_editItemDetails.m_rgchTitle ).compare( pTitleLine->text() ) ) )
		qInfo() << SteamUGC()->SetItemTitle( hUpdateHandle, pTitleLine->text().toStdString().c_str() );

	if ( !pDescLine->toPlainText().isEmpty() )
		if ( !m_edit || ( m_edit && QString( m_editItemDetails.m_rgchDescription ).compare( pDescLine->toPlainText() ) ) )
			qInfo() << SteamUGC()->SetItemDescription( hUpdateHandle, pDescLine->toPlainText().toStdString().c_str() );

	ERemoteStoragePublishedFileVisibility visibility; // AO->checkBox->isChecked() ? k_ERemoteStoragePublishedFileVisibilityPrivate : k_ERemoteStoragePublishedFileVisibilityPublic;
	switch ( AO->comboBox->currentIndex() )
	{
		case 0:
			visibility = k_ERemoteStoragePublishedFileVisibilityPublic;
			break;
		case 1:
			visibility = k_ERemoteStoragePublishedFileVisibilityPrivate;
			break;
		case 2:
			visibility = k_ERemoteStoragePublishedFileVisibilityFriendsOnly;
			break;
		case 3:
			visibility = k_ERemoteStoragePublishedFileVisibilityUnlisted;
			break;
		default:
			visibility = k_ERemoteStoragePublishedFileVisibilityPublic;
			break;
	}
	SteamUGC()->SetItemVisibility( hUpdateHandle, visibility );

	char *descr = strdup( AO->textEdit->toPlainText().toStdString().c_str() );

	SteamParamStringArray_t tags {};

	std::vector<char *> charray;
	QTreeWidgetItemIterator iterator2( AO->treeWidget );
	while ( *iterator2 )
	{
		qInfo() << ( *iterator2 )->text( 0 );
		charray.push_back( strdup( ( *iterator2 )->text( 0 ).toStdString().c_str() ) );
		++iterator2;
	}
	tags.m_nNumStrings = (int32)charray.size();
	tags.m_ppStrings = (const char **)charray.data();

	qInfo() << SteamUGC()->SetItemTags( hUpdateHandle, &tags );

	for ( auto &str : charray )
		free( str );

	for ( int ind = 0; ind < iCount; ind++ )
	{
		SteamUGC()->RemoveItemPreview( hUpdateHandle, ind );
	}

	if ( defaultFileLocIMG != "./" && QFile::exists( QDir::currentPath() + "/resources/AdditionImageCurrentThumbnail.jpg" ) )
	{
		qInfo() << SteamUGC()->SetItemPreview( hUpdateHandle, QString( QDir::currentPath() + "/resources/AdditionImageCurrentThumbnail.jpg" ).toStdString().c_str() );
	}

	QTreeWidgetItemIterator iterator4( AO->ImageTree );
	while ( *iterator4 )
	{
		qInfo() << SteamUGC()->AddItemPreviewFile( hUpdateHandle, ( ( *iterator4 )->data( 0, Qt::UserRole ).toString() ).toStdString().c_str(), k_EItemPreviewType_Image );
		++iterator4;
	}

	QTreeWidgetItemIterator iterator3( AO->treeWidget_2 );
	while ( *iterator3 )
	{
		qInfo() << ( *iterator3 )->text( 0 );
		SteamUGC()->AddItemPreviewVideo( hUpdateHandle, ( *iterator3 )->text( 0 ).toStdString().c_str() );
		++iterator3;
	}

	qInfo() << "Reached Upadte Handle";
	qInfo() << QString( descr );
	SteamAPICall_t hApiSubmitItemHandle = SteamUGC()->SubmitItemUpdate( hUpdateHandle, (const char *)descr );
	m_CallResultSubmitItemUpdate.Set( hApiSubmitItemHandle, this, &CP2MapPublisher::OnSubmitItemUpdate );
	SteamHelper::StartLoopCall();
	qInfo() << "End of Function Reached!";
	free( descr );
}

void CP2MapPublisher::OnCreateItem( CreateItemResult_t *pItem, bool bFailure )
{
	if ( bFailure )
	{
		std::string errMSG = "Your workshop item could not be created, Error code: " + std::to_string( pItem->m_eResult ) + "\nfor information on this error code: https://partner.steamgames.com/doc/api/steam_api#EResult";
		QMessageBox::warning( this, "Item Creation Failure!", errMSG.c_str(), QMessageBox::Ok );
		return;
	}
	UpdateItem( pItem->m_nPublishedFileId );
}

void CP2MapPublisher::OnSubmitItemUpdate( SubmitItemUpdateResult_t *pItem, bool bFailure )
{
	qInfo() << bFailure;
	qInfo() << pItem->m_eResult;
	this->close();
}

void CP2MapPublisher::OnOldApiSubmitItemUpdate( RemoteStorageUpdatePublishedFileResult_t *pItem, bool pFailure )
{
	qInfo() << pItem->m_eResult << "\n";
}

void CP2MapPublisher::LoadExistingDetails( SteamUGCDetails_t details, uint32 index )
{
	m_editItemDetails = details;
	pFileEntry->setText( details.m_pchFileName );
	pTitleLine->setText( details.m_rgchTitle );
	pDescLine->setText( details.m_rgchDescription );
	defaultFileLocBSP = details.m_pchFileName;

	switch ( details.m_eVisibility )
	{
		case k_ERemoteStoragePublishedFileVisibilityPublic:
			AO->comboBox->setCurrentIndex( 0 );
			break;
		case k_ERemoteStoragePublishedFileVisibilityPrivate:
			AO->comboBox->setCurrentIndex( 1 );
			break;
		case k_ERemoteStoragePublishedFileVisibilityFriendsOnly:
			AO->comboBox->setCurrentIndex( 2 );
			break;
		case k_ERemoteStoragePublishedFileVisibilityUnlisted:
			AO->comboBox->setCurrentIndex( 3 );
			break;
		default:
			AO->comboBox->setCurrentIndex( 0 );
			break;
	}

	m_EditItemIndex = index;

	std::vector<std::string> vector = splitString( details.m_rgchTags, ',' );
	for ( const std::string &str : vector )
	{
		auto item = new QTreeWidgetItem( 0 );
		item->setText( 0, QString( str.c_str() ) );
		if ( str == "Singleplayer" || str == "Cooperative" || str == "Custom Visuals" )
			item->setDisabled( true );
		AO->treeWidget->addTopLevelItem( item );
	}

	AO->disableTagWidget( false );

	std::string dir = QDir::currentPath().toStdString() + "/resources/" + std::to_string( details.m_nPublishedFileId ) + "_Image0.png";
	SteamAPICall_t res = SteamRemoteStorage()->UGCDownloadToLocation( details.m_hPreviewFile, dir.c_str(), 0 );
	m_CallOldApiResultSubmitItemDownload.Set( res, this, &CP2MapPublisher::OnOldApiSubmitItemDownload );
	SteamHelper::StartLoopCall();
	UGCQueryHandle_t hQueryResult = SteamUGC()->CreateQueryUserUGCRequest( SteamUser()->GetSteamID().GetAccountID(), k_EUserUGCList_Published, k_EUGCMatchingUGCType_Items_ReadyToUse, k_EUserUGCListSortOrder_CreationOrderDesc, SteamUtils()->GetAppID(), CP2MapMainMenu::ConsumerID, 1 );
	SteamUGC()->SetReturnAdditionalPreviews( hQueryResult, true );
	SteamAPICall_t hApiQueryHandle = SteamUGC()->SendQueryUGCRequest( hQueryResult );
	m_CallResultSendQueryUGCRequest.Set( hApiQueryHandle, this, &CP2MapPublisher::OnSendQueryUGCRequest );
	SteamHelper::StartLoopCall();
	SteamUGC()->ReleaseQueryUGCRequest( hQueryResult );
}

void CP2MapPublisher::OnSendQueryUGCRequest( SteamUGCQueryCompleted_t *pQuery, bool bFailure )
{
	qInfo() << "testing";

	SteamUGCDetails_t pDetails {};
	SteamUGC()->GetQueryUGCResult( pQuery->m_handle, m_EditItemIndex, &pDetails );
	// qInfo() << pDetails.m_flScore;

	iCount = SteamUGC()->GetQueryUGCNumAdditionalPreviews( pQuery->m_handle, m_EditItemIndex );
	int imageIndex = 0;
	for ( uint32 i = 0; i < iCount; i++ )
	{
		const uint iUrlSize = 2000;
		char pchUrl[iUrlSize];
		const uint iFileSize = 2000;
		char pchFileName[iFileSize];
		EItemPreviewType pType;

		qInfo() << SteamUGC()->GetQueryUGCAdditionalPreview( pQuery->m_handle, m_EditItemIndex, i,
															 pchUrl, iUrlSize, pchFileName,
															 iFileSize, &pType );
		auto *pItem = new QTreeWidgetItem( 0 );

		if ( pType == k_EItemPreviewType_Image )
		{
			qInfo() << pchFileName;
			qInfo() << pchUrl;
			QNetworkAccessManager manager;
			QNetworkReply *reply = manager.get( QNetworkRequest( QUrl( pchUrl ) ) );
			QEventLoop loop;
			QObject::connect( reply, SIGNAL( finished() ), &loop, SLOT( quit() ) );
			QObject::connect( reply, SIGNAL( error( QNetworkReply::NetworkError ) ), &loop, SLOT( quit() ) );
			loop.exec();

			QImage image;
			image.loadFromData( reply->readAll() );
			QByteArray ba;
			QBuffer buffer( &ba );
			buffer.open( QIODevice::ReadWrite );
			image.scaled( 1914, 1078, Qt::KeepAspectRatio );
			image.save( &buffer, "JPG" );
			QString filepath = QString( QDir::currentPath() + "/resources/AdditionImage" + QString( std::to_string( i ).c_str() ) + ".jpg" );
			QFile file( filepath );
			file.open( QIODevice::ReadWrite );
			file.write( ba );
			file.close();

			pItem->setText( 0, QString( pchFileName ) );
			pItem->setData( 0, Qt::UserRole, filepath );
			pItem->setData( 1, Qt::UserRole, imageIndex );
			AO->ImageTree->addTopLevelItem( pItem );

			delete reply;
			imageIndex++;
		}
		if ( pType == k_EItemPreviewType_YouTubeVideo )
		{
			pItem->setText( 0, QString( pchUrl ) );
			AO->treeWidget_2->addTopLevelItem( pItem );
			break;
		}
	}

	SteamHelper::FinishLoopCall();
}

void CP2MapPublisher::OnOldApiSubmitItemDownload( RemoteStorageDownloadUGCResult_t *pItem, bool pFailure )
{
	std::string filepath = QDir::currentPath().toStdString() + "/resources/" + std::to_string( m_editItemDetails.m_nPublishedFileId ) + "_Image0.png";
	QPixmap tempMap = QPixmap( filepath.c_str() );
	if ( tempMap.isNull() )
		tempMap = QPixmap( ":/zoo_textures/InvalidImage.png" );
	tempMap = tempMap.scaled( 240, 240, Qt::IgnoreAspectRatio );
	pImageLabel->setPixmap( tempMap );
	SteamHelper::FinishLoopCall();
}

void CP2MapPublisher::onAgreementButtonPressed()
{
	QDesktopServices::openUrl( QUrl( "https://store.steampowered.com/subscriber_agreement/" ) );
}

void CP2MapPublisher::OpenImageFileExplorer()
{
	QString filePath = QFileDialog::getOpenFileName( this, "Open", defaultFileLocIMG, "*.png *.jpg", nullptr, FILE_PICKER_OPTS );
	QString fPathOG = filePath;
	if ( filePath.isEmpty() )
		return;
	QPixmap tempMap = QPixmap( filePath );
	if ( tempMap.isNull() )
	{
		filePath = ":/zoo_textures/InvalidImage.png";
		tempMap = QPixmap( ":/zoo_textures/InvalidImage.png" );
	}
	tempMap = tempMap.scaled( 240, 240, Qt::IgnoreAspectRatio );
	pImageLabel->setPixmap( tempMap );

	if ( tempMap.isNull() )
		return;
	// QImage image(filePath);
	QPixmap thumbnail( filePath );
	if ( !QDir( "resources" ).exists() )
		QDir().mkdir( "resources" );

	QString filepath = QString( QDir::currentPath() + "/resources/AdditionImageCurrentThumbnail.jpg" );
	if ( thumbnail.save( filepath, "jpg" ) ){
		if ( QFileInfo(filepath).size() > 1048576 )
		{
			tempMap = QPixmap( ":/zoo_textures/InvalidImage.png" );
			tempMap = tempMap.scaled( 240, 240, Qt::IgnoreAspectRatio );
			pImageLabel->setPixmap( tempMap );
			QMessageBox::warning( nullptr, "Image File Size Too Big", "Your image exceeds the max upload limit of 1MB, the uploader's compressor was unable to compress your image to 1MB and therefore this image can't be uploaded.", QMessageBox::Ok );
			return;
		}
		defaultFileLocIMG = fPathOG;
	}

}

void CP2MapPublisher::OpenBSPFileExplorer()
{
	QString folderPath = QFileDialog::getExistingDirectory( nullptr, "Open", QDir::currentPath() );
	defaultFileLocBSP = folderPath;
	if ( folderPath.isEmpty() )
		return;

	AO->disableTagWidget( false );
	pFileEntry->setText( folderPath );
}

void CP2MapPublisher::onOKPressed()
{
	if ( pTitleLine->text().isEmpty() )
	{
		QMessageBox::warning( this, "File Required!", "You don't have a Title, please insert a title.", QMessageBox::Ok );
		return;
	}

	if ( !m_edit && ( defaultFileLocIMG == "./" || !QFile::exists( defaultFileLocIMG ) ) )
	{
		QMessageBox::warning( this, "Preview Image Required!", "You don't have a Preview Image, please insert a Preview Image", QMessageBox::Ok );
		return;
	}

	if ( !pSteamToSAgreement->isChecked() )
	{
		QMessageBox::warning( this, "Steam Workshop Contribution Agreement", "You need to accept the Steam Workshop Contribution Agreement to upload your map!", QMessageBox::Ok );
		return;
	}

	QDir dir = QDir( defaultFileLocBSP );
	if ( !m_edit && !dir.exists( defaultFileLocBSP ) )
	{
		QMessageBox::warning( this, "No Content Found!", "The specified Content Folder does not exist!", QMessageBox::Ok );
		return;
	}

	if ( m_edit )
		LoadEditItem();
	else
		LoadCreatingItem();
}

void CP2MapPublisher::LoadEditItem()
{
	UpdateItem( m_editItemDetails.m_nPublishedFileId );
}

void CP2MapPublisher::onClosePressed()
{
	this->close();
}

void CP2MapPublisher::closeEvent( QCloseEvent *event )
{
	emit mapPublisherClosed();
	event->accept();
}

std::vector<std::string> CP2MapPublisher::splitString( const std::string &input, char delimiter )
{
	std::stringstream ss { input };
	std::vector<std::string> out;
	std::string token;
	while ( std::getline( ss, token, delimiter ) )
		out.push_back( token );
	return out;
}