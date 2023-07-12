#include <iostream>
#include <time.h>
#include <vector>
using namespace std;

#include <WinSock2.h>
#include <WS2tcpip.h>
#include <Windows.h>

#pragma comment(lib, "Ws2_32.lib")

const int BUF_SIZE = 64;


#pragma pack(1)
enum damageTypes
{
	FIRE,
	ICE,
	WATER,
	NORMAL,
	ALL,
	NONE
};

enum targetTypes
{
	SELF,
	ALLIES,
	ENEMIES,
	TARGET,
	ALLTARGETS
};

struct Entity
{
	char name[20];
	int health;
	int diceSize;
	int DiceRolled;
	int ArmorClass;
	int ToHit;
	damageTypes damageClass;
};

struct Weapon
{
	char name[20];
	int diceSize;
	int DiceRolled;
	int ToHit;
	damageTypes damageClass;
};

struct Armor
{
	char name[20];
	int ArmorClass;
	damageTypes resistance;
};

struct Spell
{
	char name[20];
	char desc[50];
	targetTypes target;
	damageTypes damageClass;
	int diceSize = 0;
	int DiceRolled;
	int ToHit;
	bool isHealing;
};

struct ClassType
{
	char name[20];
	int StartingHP;
	Weapon startingWeapon;
	Armor startingArmor;
	Spell startingSpell;
	bool CanCast;
	bool hasSecondAction;
};

struct Player
{
	char name[32];
	ClassType playersClass;
	int health;
	Weapon currentWeapon;
	Armor currentArmor;
	Spell spellist[10];
	char currentAction[20];
	bool taunting = false;
	bool isMyTurn = false;
};

struct Room
{
	char name[20];
	char desc[180];
	int EnemyCount;
	Entity enemyType[10];
};

struct Player_deets {
	char name[32];
	int choice;
};
#pragma pop

void AdvancePlayerTurn(int vecIndex, vector<Player> &theParty);
int DiceRoll(int diceCount, int diceSize);
void PlayerAttack(Room &CurrentRoom, Player currentPlayer, bool &inTheRoom);
void DealDamage(Entity currentMonster, vector<Player> theParty);
void Spellcasting(Player currentPlayer, vector<Player> theParty, Room CurrentRoom);
void RollForLoot(Weapon WeaponList[], Armor ArmorList[], Spell SpellList[], vector<Player> TheParty);

int main()
{
	SOCKET sock; //New
	SOCKADDR_IN socketInfo; //New

	//This holds info about client who sent message
	SOCKADDR_IN msgContainer; //New
	int msgSize = sizeof(msgContainer);

	int iWSAStatus;
	WSADATA wsaData;
	char buf[BUF_SIZE];

	iWSAStatus = WSAStartup(MAKEWORD(2, 2), &wsaData);

	//Use IPv4 and UDP 
	sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP); //New

	//New
	socketInfo.sin_family = AF_INET;
	socketInfo.sin_port = htons(49152); //49153 port for sending
	inet_pton(AF_INET, "127.0.0.1", &(socketInfo.sin_addr));

	int sockErr = bind(sock, (sockaddr*)&socketInfo, sizeof(socketInfo));
	if (sockErr == SOCKET_ERROR)
	{
		closesocket(sock);
		cout << "Error connecting: " << WSAGetLastError() << endl;

		WSACleanup();
		return 1;
	}

	Entity EnemyArray[7] = {
		{"Goblin", 2, 6, 1, 10, 3, NORMAL},
		{"Zombie", 3, 4, 1, 13, 1, NORMAL},
		{"Orc", 4, 8, 1, 14, 4, NORMAL},
		{"Ogre", 4, 6, 2, 15, 6, NORMAL},
		{"Elemental", 5, 4, 4, 10, 5, WATER},
		{"Kobold", 6, 4, 2, 8, 4, NORMAL},
		{"Dragon", 8, 6, 5, 18, 8, FIRE} };
	Room RoomList[7] = {
		{"Lair", "In the flame wreathed halls, the foul Red Dragon sits in the center of a large pile of gold. Without thinking, it turns to you, and attacks", 1, {EnemyArray[4]}},
		{"Tomb", "A small tomb to a forgotten platoon seems to have been disturbed, especially when the writhing hordes of the undead emerge", 4, { EnemyArray[1], EnemyArray[1], EnemyArray[1], EnemyArray[1]}},
		{"Camp", "A small encampment is built in the heart of the dungeon. Before you can properly investigate, a small warband of orcs returns home, and they are unpleased by your presence", 3, {EnemyArray[2], EnemyArray[2], EnemyArray[2]}},
		//{"Lair of tresures", }
	};
	Weapon WeaponList[6] = {
		{"Dagger", 4, 1, 4, NORMAL},
		{"Arcane Dagger", 6, 1, 5, FIRE},
		{"Mace", 4, 2, 4, NORMAL},
		{"Greatsword", 10, 1, 4, NORMAL},
		{"Greatersword", 12, 2, 5, NORMAL},
		{"Greatestsword", 12, 3, 6, FIRE}
	};
	Armor ArmorList[6] = {
		{"Common Clothes", 12, NONE},
		{"Leather", 14, NONE},
		{"Studded Leather", 15, NONE},
		{"Chain", 16, NONE},
		{"Plate", 18, NORMAL},
		{"Arcane", 20, FIRE}
	};
	Spell SpellList[9] = {
		{"Firebolt", "A single ball of fire, simple, but controlled.", TARGET, FIRE, 12, 1, 5, false},
		{"Healing Word", "A simple spell that can heal you or an ally.", TARGET, FIRE, 8, 3, 5, true},
		{"Icebolt", "A shard of ice thrown at the target.", TARGET, ICE, 12, 2, 5, false},
		{"Guided Bolt", "An attack that's guarenteed to hit.", TARGET, NORMAL, 12, 1, 20, false},
		{"Flamewave", "A large wave of fire that hits everyone", ALLTARGETS, FIRE, 10, 3, 4, false},
		{"Fireball", "JUST FIREBALL", ENEMIES, FIRE, 6, 8, 7, false},
		{"Supreme Heal", "An incredibly strong healing spell", TARGET, FIRE, 12, 4, 5, true},
		{"Wave of Pain", "A spell that washes everyone away", ALLTARGETS, WATER, 10, 4, 5, false},
		{"Rally", "Heals everyone in the party.", ALLIES, FIRE, 8, 3, 5, true}
	};
	ClassType Classes[4] = {
		{"Warrior", 150, WeaponList[3], ArmorList[3], NULL, false, false},
		{"Mage", 100, WeaponList[0], ArmorList[0], SpellList[0], true, false},
		{"Thief", 75, WeaponList[0], ArmorList[1], NULL, false, true},
		{"Cleric", 125, WeaponList[0], ArmorList[3], SpellList[0], true, false},
	};

	//Game State Variables
	bool yourTurn = true;
	bool lockedIn = true;
	bool inTheRoom = true;
	bool stillEnemies = true;
	bool isLobby = true;
	string currentAction;
	int target;
	int rooms = 3;
	int chosenSpell;

	//Remember/store ip and port struct of each signed on client
	vector<SOCKADDR_IN> clients;
	bool exists = false;

	vector<Player> playerList;
	Player_deets ply{};

	//Signs in players to keep track of
	while (isLobby)
	{

		//First, wait to recieve new player info struct...
		int bytesRecv = recvfrom(sock, (char*)&ply, sizeof(ply), 0, (sockaddr*)&msgContainer, &msgSize);
		if (bytesRecv == SOCKET_ERROR)
		{
			closesocket(sock);
			cout << "Error recv: " << WSAGetLastError() << endl;
			WSACleanup();
			return 0;
		}

		//Check any/all existing info structs for match
		for (SOCKADDR_IN a : clients)
		{
			if (a.sin_port == msgContainer.sin_port) exists = true;
		}

		//If it hasn't seen this unique client yet, save its info in vector
		if (!exists) clients.push_back(msgContainer);

		//Before treating struct as real player, see if it's the dummy one
		//_stricmp returns 0 (false) if matching
		if (!_stricmp(ply.name, (char*)"start"))
		{
			isLobby = false;
		}
		else
		{
			Player player = {};

			//Send client associated with player struct (let client know if it's player 1)
			if (playerList.size() <= 0) {
				//Send first in buffer to client
				char str[] = "first";

				//Send to client who is currently signing on
				sockErr = sendto(sock, str, strlen(str) + 1, 0, (sockaddr*)&msgContainer, sizeof(msgContainer));
				if (sockErr == SOCKET_ERROR)
				{
					closesocket(sock);
					WSACleanup();
					cout << "Error connecting: " << WSAGetLastError() << endl;
					return 1;
				}

				//First player in starts first
				player.isMyTurn = true;
			} else {
				char str[] = "other";

				//Send to client who is currently signing on
				sockErr = sendto(sock, str, strlen(str) + 1, 0, (sockaddr*)&msgContainer, sizeof(msgContainer));
				if (sockErr == SOCKET_ERROR)
				{
					closesocket(sock);
					WSACleanup();
					cout << "Error connecting: " << WSAGetLastError() << endl;
					return 1;
				}
			}

			strncpy_s(player.name, ply.name, sizeof(ply.name));

			switch (ply.choice)
			{
			case 1:
				player.playersClass = Classes[0];
				player.health = Classes[0].StartingHP;
				player.currentWeapon = Classes[0].startingWeapon;
				player.currentArmor = Classes[0].startingArmor;
				player.spellist[0] = Classes[0].startingSpell;
				break;
			case 2:
				player.playersClass = Classes[1];
				player.health = Classes[1].StartingHP;
				player.currentWeapon = Classes[1].startingWeapon;
				player.currentArmor = Classes[1].startingArmor;
				player.spellist[1] = Classes[1].startingSpell;
				break;
			default:
				break;
			}

			//Add player to cpp vector
			playerList.push_back(player);
			//player = {}; //clear
		}
	}//END OF LOBBY GAME LOOP

	//Tell all players the game has started
	for (SOCKADDR_IN a : clients)
	{
		char str[] = "The game has started!";
		sendto(sock, str, strlen(str) + 1, 0, (sockaddr*)&a, sizeof(a));
	}

	//Game loop (finite room number)
	for (size_t k = 0; k < rooms; k++)
	{
		srand((unsigned)time(0));
		int ChosenRoom = rand() % 3;
		cout << "######################################################" << endl;
		cout << "Location: " << RoomList[ChosenRoom].name << endl;
		cout << RoomList[ChosenRoom].desc << endl;
		cout << "######################################################" << endl;

		inTheRoom = true;

		while (inTheRoom)
		{
			//Enemies that are still alive deal damage to players
			for (int i = 0; i < RoomList[ChosenRoom].EnemyCount; i++)
			{
				if (RoomList[ChosenRoom].enemyType[i].health > 0)
				{
					DealDamage(RoomList[ChosenRoom].enemyType[i], playerList);
				}
			}

			for (size_t i = 0; i < playerList.size(); i++)
			{
				if (playerList[i].currentAction == "Dodge" || playerList[i].currentAction == "dodge")
				{
					playerList[i].currentArmor.ArmorClass -= 5;
				}

				playerList[i].taunting = false;

				while (playerList[i].isMyTurn)
				{
					//Que specific client for their turn (give client the nod to get input)
					//Go through all clients, and if index matches send isturn
					for (SOCKADDR_IN a : clients)
					{
						//If send input nod to client whose turn it is
						if (a.sin_port == clients[i].sin_port) {
							char str[] = "isturn";
							sockErr = sendto(sock, str, strlen(str) + 1, 0, (sockaddr*)&a, sizeof(a));
							if (sockErr == SOCKET_ERROR)
							{
								closesocket(sock);
								WSACleanup();
								cout << "Error connecting: " << WSAGetLastError() << endl;
								return 1;
							}
						}
						else {
							//Otherwise tell the rest to chill out/spectate
							char str[] = "wait";
							sockErr = sendto(sock, str, strlen(str) + 1, 0, (sockaddr*)&a, sizeof(a));
							if (sockErr == SOCKET_ERROR)
							{
								closesocket(sock);
								WSACleanup();
								cout << "Error connecting: " << WSAGetLastError() << endl;
								return 1;
							}
						}
					}

					//Recieve input from all clients to make turn decision
					for (SOCKADDR_IN a : clients)
					{
						int sz = sizeof(a);
						sockErr = recvfrom(sock, buf, sizeof(buf), 0, (sockaddr*)&a, &sz);
						if (sockErr == SOCKET_ERROR)
						{
							closesocket(sock);
							WSACleanup();
							cout << "Error connecting: " << WSAGetLastError() << endl;
							return 1;
						}

						//_stricmp returns 0 (false) if matching
						if (!_stricmp(buf, (char*)"spec"))
						{
							currentAction = "spectate";
						}
						else {
							currentAction = buf;
						}
					}

					//Boot out of this while loop only for spectators
					if (currentAction == "spectate") {
						break;
					}

					if (sockErr == SOCKET_ERROR)
					{
						closesocket(sock);
						WSACleanup();
						cout << "Error connecting: " << WSAGetLastError() << endl;
						return 1;
					}

					if (currentAction == "Attack" || currentAction == "attack")
					{
						PlayerAttack(RoomList[ChosenRoom], playerList[i], inTheRoom);

						//End turn and move to next player in line
						AdvancePlayerTurn(i, playerList);
					}
					else if (currentAction == "Stats" || currentAction == "stats")
					{
						cout << "Name :	\t \t \t" << playerList[i].name << endl;
						cout << "Class : \t \t \t" << playerList[i].playersClass.name << endl;
						cout << "Health :	\t \t " << playerList[i].health << endl;
						cout << "Weapon :	\t \t " << playerList[i].currentWeapon.name << endl;
						cout << "To Hit :	\t \t " << playerList[i].currentWeapon.ToHit << endl;
						cout << "Damage :	\t \t " << playerList[i].currentWeapon.DiceRolled << "d" << playerList[i].currentWeapon.diceSize << endl;
						cout << "Armor :	\t \t \t " << playerList[i].currentArmor.ArmorClass << endl;
						cout << "Resistances :	\t \t " << playerList[i].currentArmor.resistance << endl;
					}
					else if (currentAction == "Dodge" || currentAction == "dodge")
					{
						playerList[i].currentArmor.ArmorClass += 5;
						cout << "You prepare to skilfully dodge an attack" << endl;

						//End turn and move to next player in line
						AdvancePlayerTurn(i, playerList);
					}
					else if (currentAction == "Taunt" || currentAction == "taunt")
					{
						cout << "You taunt your foe into attacking you.";
						playerList[i].taunting = true;
						
						//End turn and move to next player in line
						AdvancePlayerTurn(i, playerList);
					}
					else if (currentAction == "Spellcasting" || currentAction == "spellcasting")
					{
						if (playerList[i].playersClass.CanCast)
						{
							Spellcasting(playerList[i], playerList, RoomList[ChosenRoom]);

							//End turn and move to next player in line
							AdvancePlayerTurn(i, playerList);
						}
						else
						{
							cout << "You cannot cast spells.";
						}
					}
					else if (currentAction == "Help" || currentAction == "help")
					{
						cout << "Attack: Attacks a targeted foe with your current weapon" << endl;
						cout << "Spellcasting: Casts a spell from your spell list." << endl;
						cout << "Spelllist: Gets your current list of spells" << endl;
						cout << "Stats: Gets your current stats" << endl;
						cout << "Dodge: Increases your armor class temporarially" << endl;
						cout << "Taunt: While taunting, enemy attacks can only target taunting players" << endl;
					}
					else
					{
						cout << "I do not recognize that command" << endl;
					}//End of player options
				}//End player turn


			}//End of for each player
		}//End of while in room

		//RollForLoot(WeaponList, ArmorList, SpellList, playerList);

	}//End of for each room loop

	cout << "You have cleared the dungeon. Victory is yours! Thanks for playing!" << endl;

	return 0;
}

//Circularly cycle turn to next player in vector
void AdvancePlayerTurn(int vecIndex, vector<Player> &theParty) {

	//End current players turn
	theParty[vecIndex].isMyTurn = false;

	//Is current player last in vector
	if (vecIndex == (theParty.size() - 1)) {
		//Set turn of first player back to true
		theParty[0].isMyTurn = true;
	}
	else 
	{
		//Otherwise if there is a next in line, set to true
		theParty[vecIndex + 1].isMyTurn = true;
	}
}

int DiceRoll(int diceCount, int diceSize)
{
	int roll = 0;
	for (size_t i = 0; i < diceCount; i++)
	{
		roll = (rand() % diceSize) + 1;
	}
	return roll;
}

void DealDamage(Entity currentMonster, vector<Player> theParty)
{
	bool AllTargetsValid = true;
	for (size_t i = 0; i < theParty.size(); i++)
	{
		if (theParty[i].taunting)
		{
			AllTargetsValid = false;
		}
	}

	int HitRoll = DiceRoll(1, 20) + currentMonster.ToHit;
	cout << "******************************************************" << endl;
	cout << "The " << currentMonster.name << " rolls a " << HitRoll << " ";
	int damage = DiceRoll(currentMonster.DiceRolled, currentMonster.diceSize);
	if (AllTargetsValid)
	{
		for (size_t i = 0; i < theParty.size(); i++)
		{
			if (HitRoll >= theParty[i].currentArmor.ArmorClass)
			{
				if (theParty[i].currentArmor.resistance == currentMonster.damageClass || theParty[i].currentArmor.resistance == ALL)
				{
					theParty[i].health -= damage / 2;
					cout << "The " << currentMonster.name << " hits " << theParty[i].name << ", rolling " << damage << "resisted damage" << endl;
				}
				else
				{
					theParty[i].health -= damage;
					cout << "The " << currentMonster.name << " hits " << theParty[i].name << ", rolling " << damage << " damage" << endl;
				}
			}
			else
			{
				cout << "The " << currentMonster.name << " missed " << theParty[i].name << "!" << endl;
			}
		}
	}
	else
	{
		for (size_t i = 0; i < theParty.size(); i++)
		{
			if (theParty[i].taunting)
			{
				if (HitRoll >= theParty[i].currentArmor.ArmorClass)
				{
					if (theParty[i].currentArmor.resistance == currentMonster.damageClass || theParty[i].currentArmor.resistance == ALL)
					{
						theParty[i].health -= damage / 2;
						cout << "The " << currentMonster.name << " hits " << theParty[i].name << ", rolling " << damage << "resisted damage" << endl;
					}
					else
					{
						theParty[i].health -= damage;
						cout << "The " << currentMonster.name << " hits " << theParty[i].name << ", rolling " << damage << " damage" << endl;
					}
				}
				else
				{
					cout << "The " << currentMonster.name << " missed " << theParty[i].name << "!" << endl;
				}
			}
		}
	}
	cout << "******************************************************" << endl;
}

void PlayerAttack(Room &CurrentRoom, Player currentPlayer, bool &inTheRoom)
{
	bool lockedIn = true;
	int target;
	if (CurrentRoom.EnemyCount == 1)
	{
		target = 1;
	}
	else
	{
		//Initial random target
		//target = (rand() % CurrentRoom.EnemyCount) + 1;
		target = 1;

		while (CurrentRoom.enemyType[target - 1].health <= 0)
		{
			//Get new target, original was already dead
			//target = (rand() % CurrentRoom.EnemyCount) + 1;
			target++;
		}
	}
	int HitRoll = DiceRoll(1, 20) + currentPlayer.currentWeapon.ToHit;
	cout << "------------------------------------------------------" << endl;
	cout << currentPlayer.name << " rolls a " << HitRoll << " " << endl;
	if (HitRoll >= CurrentRoom.enemyType[target - 1].ArmorClass)
	{
		int damage = DiceRoll(currentPlayer.currentWeapon.DiceRolled, currentPlayer.currentWeapon.diceSize);

		CurrentRoom.enemyType[target - 1].health -= damage;

		cout << currentPlayer.name << " hits, rolling " << damage << " damage" << endl;
		cout << CurrentRoom.enemyType[target - 1].name << " remaining health: " << CurrentRoom.enemyType[target - 1].health << endl;
		cout << "------------------------------------------------------" << endl;

		if (CurrentRoom.enemyType[target - 1].health <= 0)
		{
			cout << "The " << CurrentRoom.enemyType[target - 1].name << " is dead!" << endl;
			cout << "------------------------------------------------------" << endl;
			bool stillEnemies = false;

			for (size_t i = 0; i < CurrentRoom.EnemyCount; i++)
			{
				if (CurrentRoom.enemyType[i].health > 0)
				{
					stillEnemies = true;
				}
			}

			if (!stillEnemies)
			{
				cout << "------------------------------------------------------" << endl;
				cout << "All enemies are defeated. The day is won!" << endl;
				cout << "------------------------------------------------------" << endl;
				inTheRoom = false;
			}
		}
	}
	else
	{
		cout << "------------------------------------------------------" << endl;
		cout << currentPlayer.name << " missed!" << endl;
		cout << "------------------------------------------------------" << endl;
	}
	lockedIn = false;
}

void Spellcasting(Player currentPlayer, vector<Player> theParty, Room CurrentRoom)
{
	int chosenSpell;
	int target;
	bool stillEnemies;
	cout << "Spell list: ";
	for (size_t j = 0; j < 10; j++)
	{
		if (currentPlayer.spellist[j].diceSize != 0)
		{
			cout << currentPlayer.spellist[j].name << " [" << j << "] ";
		}
	}
	cout << endl;
	cout << "Which spell do you wish to cast?: ";
	cin >> chosenSpell;
	if (currentPlayer.spellist[chosenSpell].diceSize != 0)
	{
		cout << "You don't have a spell in that slot" << endl;
	}
	else
	{
		cout << currentPlayer.name << " casts " << currentPlayer.spellist[chosenSpell].name << endl;
		if (currentPlayer.spellist[chosenSpell].target == TARGET)
		{
			bool lockedIn = true;
			while (lockedIn)
			{
				cout << "Which target?";
				cin >> target;
				if (!target || target > CurrentRoom.EnemyCount || cin.fail())
				{
					cout << "Please Insert a valid target." << endl;
				}
				else if (CurrentRoom.enemyType[target - 1].health <= 0)
				{
					cout << "That enemy is already dead";
				}
				else
				{
					lockedIn = false;
				}

			}
			int HitRoll = DiceRoll(1, 20) + currentPlayer.spellist[chosenSpell].ToHit;
			cout << currentPlayer.name << " rolls a " << HitRoll << " ";
			if (HitRoll >= CurrentRoom.enemyType[target - 1].ArmorClass)
			{
				int damage = DiceRoll(currentPlayer.spellist[chosenSpell].DiceRolled, currentPlayer.spellist[chosenSpell].diceSize);
				CurrentRoom.enemyType[target - 1].health -= damage;
				cout << currentPlayer.name << " hits, rolling " << damage << " damage" << endl;
				if (CurrentRoom.enemyType[target - 1].health <= 0)
				{
					cout << "The " << CurrentRoom.enemyType[target - 1].name << " is dead!";
					for (size_t i = 0; i < CurrentRoom.EnemyCount; i++)
					{
						if (CurrentRoom.enemyType[i].health > 0)
						{
							stillEnemies = true;
						}
					}
					if (!stillEnemies)
					{
						cout << "All enemies are defeated. The day is won!" << endl;
					}
				}
			}
		}
		else if (currentPlayer.spellist[chosenSpell].target == ENEMIES)
		{
			int HitRoll = DiceRoll(1, 20) + currentPlayer.spellist[chosenSpell].ToHit;
			cout << currentPlayer.name << " rolls a " << HitRoll << " ";
			for (size_t j = 0; j < CurrentRoom.EnemyCount; j++)
			{
				if (HitRoll >= CurrentRoom.enemyType[j].ArmorClass)
				{
					int damage = DiceRoll(currentPlayer.spellist[chosenSpell].DiceRolled, currentPlayer.spellist[chosenSpell].diceSize);
					CurrentRoom.enemyType[j].health -= damage;
					cout << currentPlayer.name << " hits, rolling " << damage << " damage" << endl;
					if (CurrentRoom.enemyType[j].health <= 0)
					{
						cout << "The " << CurrentRoom.enemyType[target - 1].name << " is dead!";
						for (size_t q = 0; q < CurrentRoom.EnemyCount; q++)
						{
							if (CurrentRoom.enemyType[q].health > 0)
							{
								stillEnemies = true;
							}
						}
						if (!stillEnemies)
						{
							cout << "All enemies are defeated. The day is won!" << endl;
						}
					}
				}
			}
		}
		else if (currentPlayer.spellist[chosenSpell].target == SELF)
		{
			int damage = DiceRoll(currentPlayer.spellist[chosenSpell].DiceRolled, currentPlayer.spellist[chosenSpell].diceSize);
			if (currentPlayer.spellist[chosenSpell].isHealing)
			{
				currentPlayer.health -= damage;
				cout << currentPlayer.name << " heals " << damage << " Hit Points" << endl;
			}
			else
			{
				currentPlayer.health -= damage;
				cout << currentPlayer.name << " hits, dealing " << damage << " damage" << endl;
			}
		}
		else if (currentPlayer.spellist[chosenSpell].target == ALLIES)
		{
			int damage = DiceRoll(currentPlayer.spellist[chosenSpell].DiceRolled, currentPlayer.spellist[chosenSpell].diceSize);
			for (size_t j = 0; j < theParty.size(); j)
			{
				if (currentPlayer.spellist[chosenSpell].isHealing)
				{
					theParty[j].health += damage;
					cout << currentPlayer.name << " heals " << damage << " Hit Points" << endl;
				}
				else
				{
					theParty[j].health -= damage;
					cout << currentPlayer.name << " hits, dealing " << damage << " damage" << endl;
				}
			}
		}
		else if (currentPlayer.spellist[chosenSpell].target == ALLTARGETS)
		{
			int HitRoll = DiceRoll(1, 20) + currentPlayer.spellist[chosenSpell].ToHit;
			cout << currentPlayer.name << " rolls a " << HitRoll << " ";
			int damage = DiceRoll(currentPlayer.spellist[chosenSpell].DiceRolled, currentPlayer.spellist[chosenSpell].diceSize);
			for (size_t j = 0; j < theParty.size(); j++)
			{
				if (HitRoll >= theParty[j].currentArmor.ArmorClass)
				{
					if (theParty[j].currentArmor.resistance == currentPlayer.spellist[chosenSpell].damageClass || theParty[j].currentArmor.resistance == ALL)
					{
						theParty[j].health -= damage / 2;
						cout << theParty[j].name << " is hit, rolling " << damage << "resisted damage" << endl;
					}
					else
					{
						theParty[j].health -= damage;
						cout << theParty[j].name << " is hit, rolling " << damage << " damage" << endl;
					}
				}
				else
				{
					cout << theParty[j].name << " dodges the attack" << endl;
				}
			}
			for (size_t j = 0; j < CurrentRoom.EnemyCount; j++)
			{
				if (HitRoll >= CurrentRoom.enemyType[j].ArmorClass)
				{
					CurrentRoom.enemyType[j].health -= damage;
					cout << "The" << CurrentRoom.enemyType[j].name << " is hit, rolling " << damage << " damage" << endl;
				}
				else
				{
					cout << "The" << CurrentRoom.enemyType[j].name << " dodges the attack" << endl;
				}
			}
		}
	}
}

void RollForLoot(Weapon WeaponList[], Armor ArmorList[], Spell SpellList[], vector<Player> TheParty)
{
	char currentAction[20];
	cout << "Rolling for loot..." << endl;
	int loot = rand() % 3;
	if (loot == 0)
	{
		Weapon lootWeapon = WeaponList[rand() % (sizeof(WeaponList) / sizeof(WeaponList[0]))];
		cout << "You found a " << lootWeapon.name << endl;
		cout << "Stats: " << endl;
		cout << "To hit: " << lootWeapon.ToHit << endl;
		cout << "Damage: " << lootWeapon.DiceRolled << "D" << lootWeapon.diceSize << " " << lootWeapon.damageClass << " damage." << endl;
		for (size_t i = 0; i < TheParty.size(); i++)
		{
			cout << TheParty[i].name << " type dibs if you want this weapon.";
			cin >> currentAction;
			if (currentAction == "dibs" || currentAction == "Dibs")
			{
				TheParty[i].currentWeapon = lootWeapon;
				cout << TheParty[i].name << " has claiemd the " << lootWeapon.name;
			}
		}
	}
	else if (loot == 1)
	{
		Armor lootArmor = ArmorList[rand() % (sizeof(ArmorList) / sizeof(ArmorList[0]))];
		cout << "You found a " << lootArmor.name << endl;
		cout << "Stats: " << endl;
		cout << "Armor class: " << lootArmor.ArmorClass;
		cout << "Resistances: " << lootArmor.resistance;
		for (size_t i = 0; i < TheParty.size(); i++)
		{
			cout << TheParty[i].name << " type dibs if you want this armor.";
			cin >> currentAction;
			if (currentAction == "dibs" || currentAction == "Dibs")
			{
				TheParty[i].currentArmor = lootArmor;
				cout << TheParty[i].name << " has claiemd the " << lootArmor.name;
			}
		}
	}
	else if (loot == 2)
	{
		Spell lootSpell = SpellList[rand() % (sizeof(SpellList) / sizeof(SpellList[0]))];
		cout << "You found a scroll of" << lootSpell.name << endl;
		cout << "Stats: " << endl;
		cout << lootSpell.desc << endl;
		for (size_t i = 0; i < TheParty.size(); i++)
		{
			if (TheParty[i].playersClass.CanCast)
			{
				cout << TheParty[i].name << " type dibs if you want this armor.";
				cin >> currentAction;
				if (currentAction == "dibs" || currentAction == "Dibs")
				{
					for (size_t j = 0; j < 10; j++)
					{
						if (TheParty[i].spellist[j].diceSize == 0)
						{
							TheParty[i].spellist[j] = lootSpell;
							cout << TheParty[i].name << " has claiemd the " << lootSpell.name;
						}
						else
						{
							cout << "You already have max spells";
						}
					}

				}
			}
		}
	}
}
