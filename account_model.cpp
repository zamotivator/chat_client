#include <TelepathyQt4/Types>
#include <TelepathyQt4/Account>
#include <TelepathyQt4/AccountFactory>
#include <TelepathyQt4/AccountManager>
#include <TelepathyQt4/PendingOperation>
#include <TelepathyQt4/PendingReady>

#include "account_model.h"

AccountModel::AccountModel(QObject *parent) :  QAbstractListModel(parent)
{
  m_account_manager= Tp::AccountManager::create();
  becomeUpdate(m_account_manager->becomeReady());
  connect(m_account_manager.data(), SIGNAL(newAccount(const Tp::AccountPtr&)),
	  this, SLOT(onNewAccount(const Tp::AccountPtr&)));
}

void AccountModel::onNewAccount(const Tp::AccountPtr &account)
{
  Q_UNUSED(account);
  qDebug() << "AccountModel: started adding";
  onFinished(0);
}

void AccountModel::onFinished(Tp::PendingOperation *)
{
  qDebug() << "AccountModel: onFinished";
  reset();
}

void AccountModel::reset()
{
  qDebug() << "Acount Model: reset";
  foreach(const Tp::AccountPtr &account, m_account_list)
  {
    account->disconnect();
  }
  m_account_list= m_account_manager->allAccounts();
  foreach(const Tp::AccountPtr &account, m_account_list)
  {
    connect(account.data(), SIGNAL(stateChanged(bool)),
	    this, SLOT(reset()));
    connect(account.data(), SIGNAL(displayNameChanged(const QString&)),
	    this, SLOT(reset()));
    connect(account.data(), SIGNAL(nicknameChanged(const QString&)),
	    this, SLOT(reset()));
  }
  QAbstractListModel::reset();
}

AccountModel::~AccountModel()
{
}

int AccountModel::rowCount(const QModelIndex &parent) const
{
  Q_UNUSED(parent);
  return m_account_list.count();
}

int AccountModel::columnCount(const QModelIndex &parent) const
{
  Q_UNUSED(parent);
  return eColumnCount;
}

void AccountModel::becomeUpdate(Tp::PendingOperation *operation)
{
  qDebug() << "AccountMode: update started";
  beginResetModel();
  connect(operation, SIGNAL(finished(Tp::PendingOperation*)),
	    this, SLOT(onFinished(Tp::PendingOperation*)));    
}
bool AccountModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
  if (!index.isValid())
  {
    return false;
  }

  if (index.row() >= rowCount())
  {
    return false;
  }

  if (index.column() >= columnCount())
  {
    return false;
  }

  if (role != Qt::EditRole || !editable(index.column()))
  {
    return false;
  }

  const Tp::AccountPtr &account= m_account_list.at(index.row());
  Q_ASSERT(editable(index.column()));

  switch(index.column())
  {
  case eEnabled:     account->setEnabled(value.toBool());       break;
  case eDisplayName: account->setDisplayName(value.toString()); break;
  case eNickName:    account->setNickname(value.toString());    break;
  default:           Q_ASSERT(false); return false;
  };

  return true;
}

bool AccountModel::editable(int column)
{
  switch(column)
  {
  case eValid:                return false;
  case eEnabled:              return true;
  case eConnectionManager:    return false;
  case eProtocolName:         return false;
  case eDisplayName:          return true;
  case eNickName:             return true;
  case eConnectAutomatically: return false;
  case eAutomaticPresence:    return false;
  case eCurrentPresence:      return false;
  case eRequestedPresence:    return false;
  case eChangingPresence:     return false;
  case eConnectionStatus:     return false;
  case eConnection:           return false;
  default:                    return 0;
  }
}

QVariant AccountModel::data(const QModelIndex &index, int role) const
{
  if (!index.isValid())
  {
    return QVariant();
  }

  if (index.row() >= rowCount())
  {
    return QVariant();
  }

  if (index.column() >= columnCount())
  {
    return QVariant();
  }

  if (role != Qt::DisplayRole && role != Qt::EditRole)
  {
    return QVariant();
  }

  const Tp::AccountPtr &account= m_account_list.at(index.row());

  if (account.isNull())
  {
    return QVariant();
  }

  switch (index.column())
  {
  case eValid:                return account->isValidAccount();
  case eEnabled:              return account->isEnabled();
  case eConnectionManager:    return account->cmName();
  case eProtocolName:         return account->protocolName();
  case eDisplayName:          return account->displayName();
  case eNickName:             return account->nickname();
  case eConnectAutomatically: return account->connectsAutomatically();
  case eAutomaticPresence:    return account->automaticPresence().status();
  case eCurrentPresence:      return account->currentPresence().status();
  case eRequestedPresence:    return account->requestedPresence().status();
  case eChangingPresence:     return account->isChangingPresence();
  case eConnectionStatus:     return account->connectionStatus();
  case eConnection:
  {
    if (account->connection().isNull())
    {
      return QVariant();
    }
    else
    {
      return account->connection()->objectPath();
    }
  }
  default:           return QVariant();
  };
}
 
QVariant AccountModel::headerData(int section, Qt::Orientation orientation, int role) const
{
  if (orientation != Qt::Horizontal)
  {
    return QVariant();
  }
    
  switch(section)
  {
  case eValid:                return tr("Valid");
  case eEnabled:              return tr("Enabled");
  case eConnectionManager:    return tr("Connection manager");
  case eProtocolName:         return tr("Protocol name");
  case eDisplayName:          return tr("Display name");
  case eNickName:             return tr("Nick name");
  case eConnectAutomatically: return tr("Connect automaticly");
  case eAutomaticPresence:    return tr("Automatic presence");
  case eCurrentPresence:      return tr("Current presence");
  case eRequestedPresence:    return tr("Request presence");
  case eChangingPresence:     return tr("Changing presence");
  case eConnectionStatus:     return tr("Connection status");
  case eConnection:           return tr("Connection");
  default:           return QVariant();      
  };
}
