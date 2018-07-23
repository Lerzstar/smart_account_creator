# Smart Account Creator for EOS

This is a smart contract for EOS specifically designed for people who have their EOS 
on an exchange and don't have their own EOS account yet. It removes the need for a third-party like a friend or
an account creation service. Since it's a smart contract, the account creation happens instantly, automatically and trustless.

This contract has been forked and modified from the original to suit the purposes of Greymass tools, and to enable maintenence and compatibility.

## How to use?
Send the recommended EOS to the contract which is deployed at the EOS account ```setupaccount```. In the memo, 
you give the desired account name, the owner public key and the active public key separated by the ```-``` character. 

For example, if your account name is ```mynewaccount```, your owner key is ```EOS6ra2QHsDr6yMyFaPaNwe3Hz8XmYRj3B68e5tbDchyPTTasgFH9``` 
and your active key is ```EOS8WcL1CroNrXfdphkohCmea1Jgp7TpqQXrkpcF1gETweeSnphmJ```, the memo string would be:

```
mynewaccount-EOS6ra2QHsDr6yMyFaPaNwe3Hz8XmYRj3B68e5tbDchyPTTasgFH9-EOS8WcL1CroNrXfdphkohCmea1Jgp7TpqQXrkpcF1gETweeSnphmJ
```

Some exchanges like Binance don't allow to use a long memo. In this case or if you use the same key for owner and active, the second key including the ```-``` separator can be omitted. 
So that would be a valid memo string as well:

```
mynewaccount-EOS6ra2QHsDr6yMyFaPaNwe3Hz8XmYRj3B68e5tbDchyPTTasgFH9
```

## How does it work?
When you withdraw your EOS to the setupaccount smart contract, it will perform the following steps in order:

1. Create a new account using your specified name, owner key and active key
1. Buy 4KB of RAM for your new account with parts of the transferred EOS. Every account that is created on the EOS network needs 4 KB of RAM to exist.
1. Delegate and transfer 0.1 EOS for CPU and 0.1 EOS for NET.
1. Forward the remaining EOS balance to your new account.

Should any of the above actions fail, the transaction will be rolled back which 
means the money will automatically be refunded to you.
