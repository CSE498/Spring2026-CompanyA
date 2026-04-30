# InteractiveWorldInventory,InteractiveWorldSaveManager

Hi, my name is Ho Wang Ho I was responsible for implementing the InteractiveWorldInventory and InteractiveWorldSaveManager classes in our Interactive World module.
The InteractiveWorldInventory class is designed to store and manage the world’s core resources, including wood, stone, and metal. It keeps track of resource totals by ItemType using a map-based structure. This class supports adding resources, removing resources, checking whether enough materials are available for upgrades, clearing the inventory, and exposing the stored data for use by the UI or other systems.

To make the inventory safer and easier to manage, I added a maximum limit for each resource type. The AddItem function prevents the total from exceeding that maximum, while RemoveItem only succeeds if enough of the requested resource is available. The GetAmount and HasEnough functions provide simple ways for other parts of the module to check resource totals and upgrade requirements.

I also implemented the InteractiveWorldSaveManager class, which is responsible for saving and loading world state. This class serializes the current inventory and building progress into a JSON file. When saving, it records the amounts of wood, stone, and metal, along with each building’s name and current level. When loading, it restores the inventory values and matches saved building data back to the existing world objects so their upgrade levels can be restored.

Overall, these two classes help support both progression and persistence in the Interactive World system. The inventory class manages the player’s stored resources, while the save manager ensures that important game progress, such as resources and building upgrades, can be preserved between runs.

