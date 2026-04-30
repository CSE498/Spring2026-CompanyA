**Short group and module intro**  
Hi we’re group 2, and we were responsible for the Classic Agents module for Green & White Game’s Slay the Dungeon (I think this is what we decided on, please correct if wrong). The goal of our module is to provide the game with rule-based agents with predictable, reusable behavior. Instead of learning based AI, our agents used predefined logic, behavior trees, action maps, pathing tools, trading logic, and world interaction hooks to support enemies, merchants, farmers, and the player interaction flow.

**Short recap of first 5 classes?**  
Get back to this

**Show overall classic agents architecture** (Maybe show like a diagram or folder tree)  
After building our initial tools, we used them to support the creation of the actual Classic Agents module. The main idea is that worlds own agents, agents choose actions, and worlds handle those actions. Agents mostly return movement or interaction action IDs, while more complex effects like trading, combat, farming, and analytics happen through agent or world interaction functions.

**Fighting \- Skeleton and Goblin in WebUI**  
\[Intro into part of demo script\]

- Goblin Flow: If player is in range and line-of-sight, attack; otherwise chase with pathfinding  
  - The goblin is meant to behave more like a melee based enemy. If the player is in range and there is a valid line of sight, the goblin can attack. Otherwise, it will try to chase the player using pathfinding. So basic flow is: check whether the player can be attacked, and if not, find a path towards the player and move one step closer.  
- Skeleton Flow: Uses bounded range behavior, can hold distance when appropriate, and has fallback attack behavior.  
  - The skeleton is meant to behave more as a range-based enemy. It uses more bounded range behavior, so it can try to keep distance when appropriate, but still has fallback attack behavior when the player gets close enough.  
- Patrol/test patterns are also supported for demos and testing

Pathing decisions use shared path tools, then map movement vectors back into registered world actions like up/down/left/right/stay.

**Merchant agent, trading system, and farming agent**  
For the non-enemy side of our module, we also implemented support for merchant and farming style agents. These agents are still rule based, but instead of focusing on combat, they focus more on player interaction and world interaction.

The MerchantAgent represents an NPC that the player can trade with. It stores a set of trade offers, tracks whether the shop is open, and keeps its own gold value separate from the player. This lets the merchant act like an actual game object rather than just a static menu. The player is able to buy items from a merchant, sell items back, and the merchant can support both limited-stock and unlimited-stock offers.

Actual buying/selling rules are handled through the TradingSystem class. I did this to keep transaction logic separate from the agent itself. The TradingSystem

**Testing**  
We covered behavior with focused tests across enemy logic 

- Agent factory setup and spawn validations  
- Movement and chase logic under different kinds of paths and line-of-sight changes  
- Attack behavior and damage expectations  
- Merchant buy/sell paths  
- Edge cases were also thoroughly handled: Zero quantity, unknown items, out-of-stock, insufficient gold, merchant closed state. 

Our tests gave us confidence that both normal and failure cases are stable and predictable  
**Interoperability with other groups**

**Wrap up and future improvements**

**Updated Script based on demo:**

**Short group and module Intro**  
Hi, we’re group 2, and this is the final project video for our Classic Agents module. The goal of our module is to provide the game with rule-based agents with predictable, reusable behavior. Instead of learning based AI, our agents used predefined logic, behavior trees, action maps, pathing tools, trading logic, weapon handling, and world interaction hooks. These components work together to support enemies like goblins and skeletons, merchant trading, player combat, inventory management, and other interaction systems used in the game.

**Overall architecture**  
After building our initial tools, we used them to support the creation of the actual Classic Agents module. The main idea is that worlds own agents, agents choose actions, and worlds handle those actions. Agents mostly return movement or interaction action IDs, while more complex effects like trading, combat, farming, and analytics happen through agent or world interaction functions.

While our npc and player controlled agents are defined elsewhere, we decided it would be simpler to have most of our agent creation be handled through one central factory class. When instantiating one of these agents, the factory sets its stats and creates its corresponding behavior tree.

**Skeleton Slide 6**  
Our skeleton is the positional enemy out of the two. It attacks from a bounded band: It lines up a clear shot and fights from maximum reach, as far away as its range allows, instead of stacking on you. If that fails, the skeleton either paths closer when you are far, steps back when you are too close, or stands still if it sees you but has no clear shot. The retreat function uses a blackboard step counter capped at five; after that it stops kiting and attacks again.   
**Skeleton WebUI demo Slide 7**  
For the WebUI demo , we ported the same world for color and readability. In the demo take notice of how the skeleton tries to open distance, back off in short 5 step bursts and then commit to attacking when that step limit hits or the path tightens.  
**Goblin Slide 8**  
The goblin is a much simpler design, it uses the same repeat to selector pattern, but the first branch in the tree is just IsPlayerInRange, which is determined by line of sight plus distance, then attacks the player. If that happens to fail, ChasePlayer takes one shortest-path step toward you each tick. Unlike the skeleton and due to it being melee, there is no sweet spot determined by its movement. So if there were to be a similar demo you would see that the goblin is there to put straight line pressure compared to the skeletons' more strategical gameplay. 

**Trading in our demo**  
MerchantAgent Slide:  
For the non-enemy side of our module, we also implemented support for trading with merchant and farming style agents. These agents are still rule based, but instead of focusing on combat, they focus more on player interaction and inventory interaction.

The MerchantAgent represents an NPC that the player can trade with. It stores a set of trade offers, tracks whether the shop is open, and keeps its own gold value separate from the player. This lets the merchant act like an actual game object rather than just a static menu. The player is able to buy items from a merchant, sell items back, and the merchant can support both limited-stock and unlimited-stock offers. So some items can run out, while others can keep being purchased repeatedly.

TradeSystem Slide:  
The actual buying and selling rules are handled through the TradeSystem class. This is to keep transaction logic separate from the merchant agent itself, so the merchant can focus on storing offers and state, while the TradeSystem handles all the transaction rules. 

It checks for things like whether the item exists, whether the quantity is valid, whether the player has enough gold, whether the merchant has enough stock, and whether the player has room in their inventory. 

This helps invalid trades fail cleanly. For example, if the player doesn’t have enough gold, the transaction doesn’t partially remove stock from the merchant. Then if the player’s inventory is full, the system rolls back the purchase instead of leaving the game in an inconsistent state.

Merchant Demo Slide:  
For the demo, a full trading menu UI is not yet supported, so we are showing the trading behavior in the Group 2 demo flow instead. 

First, the player walks next to the agent and uses the interact action. That triggers trade with the merchant. And we attempt to buy apples from the merchants available trade offers. We then interact again to buy some bread from the merchant. After making these purchases we can see that the trade system successfully handled the trade interaction and added the items to the player’s inventory. 

FarmingAgent Slide:  
The FarmingAgent builds on the merchant-style system, but adds some worker behavior. It extends MerchantAgent, so it can still support trading, but it also has a home position, an assigned building, a work interval, and optional restock behavior.   
Its basic loop is that the farmer waits at home, travels to its assigned building when it is time to work, interacts with the building when it gets close enough, and then returns home afterward. 

This was designed for integration with the Interactive World group in mind. The world can decide what actually happens when the farmer works at the building, while our agent handles the behavior pattern of when to go there, when to interact, and when to return. For example, after the farmer successfully works at a building, it can restock one of its limited merchant offers, which connects the farming behavior back to the trading system, because the farmer can act both like a worker NPC and a merchant NPC. While this agent is implemented and tested, we do not have a demo since the FetchAgent was created and used instead for a similar purpose.

**Weapons**  
We handled the implementation of weapons while group 15 created and decided the overall categories. In our system, weapons are treated as a specialized type of item that can modify the player’s combat stats when equipped.

Each weapon stores values like damage and range. When the player’s held hotbar item changes, the player’s combat stats are automatically refreshed based on that item. If the item is a valid weapon, its damage is added to the player’s base attack, and its range becomes the player’s attack range. For example, the sword has higher damage but short range, while the bow has lower damage but longer range.

If the player is not holding a weapon or if the item is just a tool, then the player keeps their base combat stats. This keeps base stats and equipment effects separate, while still integrating weapons directly into the combat system. This design also allows stats to update dynamically as the player switches items, without needing to manually reconfigure combat values. It keeps weapons modular, so new weapon types can be added easily without changing the core combat system.  

**Inventory System**

As you saw in the weapons demo and the merchant demo, the player can manage his inventory to use items or hold goods.

We tried to replicate that of minecraft or similar. The hotbar has 10 slots with a 4x5 grid for the backpack with each slot able to hold multiple items. We chose an std::array to build the backpack so that it isn’t on the heap as it is a statically sized object. We put a lot of consideration was made when designing the API such as putting slots 0-9 as the hotbar for easy indexing 

And starting insertion and removal at the top left of the inventory (slot 10\) then making a full loop back to to slot 10 so the inventory bar doesn’t get cluttered

THe function for addition and removal of items return the amount that wasn’t added or the quantity that wasn’t removed so that both operations are successful if returns are 0 which makes success easy and removal/addition counts convenient. 

We also had a “all or nothing” parameter for removal to make trading easier – successfully completing the trade only when the player has sufficient items 

To make this whole thing efficient, we have an unordered item map, mapping from name to location index so we have fast insertion, indexing, and removal into the array

**PathGenerator**

We have two other important new functions from path generator: 

1\. We added a function IsPathClear to determine if there is a CLEAR LINE OF SIGHT FROM the start position along some path vector to the tile where another agent is. We further added special checks for tile corner intersections making sure both perpendicularly adjacent tiles are not walls. 

We also added Find furthest point which is an efficient algorithm for the skeleton to find a path away from the player while allowing it to be trapped if the player approaches from the right angle – like if approaching the skeleton from a diagonal then the skeleton will be trapped in a corner. 

So I feel we have captured the essence of what classic agents are meant to do with decent structure and expandability. While not everything was able to be completed perfectly as we intended. We learned a lot along the way.   
