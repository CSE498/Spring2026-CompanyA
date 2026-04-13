# BSP Dungeon Generation Dump (Given The BSP is generated with seed 12345)
# Data information generated from BSP-Dungeon's TreeParser() function, Formatted with help from ChatGPT


###### Some clarification #######
The number next to the node represent the hierarchy in the tree. The highest (#) value of the node (ex. Node #) is the root node
and the descending (#) of the Nodes represent the depth level of node. Nodes of the same (#) value are on the same depth level.

## High Level Abstraction representation of the tree 
Container 
# BSP Tree (Clean View)

# Total Leaf + Node Count in BSP Tree
# Leaf: 29
# Tree: 15

```
Container (Generated with seed 12345)
├── Left (Node 20)
│   ├── 19
│   │   ├── 18
│   │   │   ├── 17
│   │   │   │ 
│   │   │   │  
│   │   │   └── 17
│   │   └── 18
│   │       ├── 17
│   │       │  
│   │       │  
│   │       └── 17  
│   │     
│   └── 19
│
└── Right (Node 20)
    ├── 19
    │   ├── 18
    │   │   ├── 17
    │   │   └── 17
    │   └── 18
    └── 19
```

## Node Format ##
Name: String
Position: (x, y)
Size: width x height
Left Child: int indices
Right Child: int indices

###### Node Info Below ######

## Root Node

```
Name: Container
Position: (0, 0)
Size: 150 x 100
Left Child: 1
Right Child: 22
```

---

## Node 20

```
Position: (0, 0)
Size: 93 x 100
Left Child: 2
Right Child: 17
```

---

## Node 19

```
Position: (0, 0)
Size: 93 x 71
Left Child: 3
Right Child: 8
```

---

## Node 18

```
Position: (0, 0)
Size: 93 x 27
Left Child: 4
Right Child: 7
```

---

## Node 17

```
Position: (0, 0)
Size: 60 x 27
Left Child: 5
Right Child: 6
```

---

## Leaf Node 16

```
Position: (0, 0)
Size: 30 x 27
```

### Room Layout

```
^^^^^^^^^^^^^^^^^^^^^^^^^^
<   $$                   >
<                        >
<       $$      $$       >
<       $$      $$       >
<       $$      $$       >
<           m            >
<       $$      $$       >
<       $$      $$       >
<                        >
<       $$      $$       >
<                        >
<                        >
&&&&&&&&&&&&&&&&&&&&&&&&&&
```

---

## Leaf Node 16

```
Position: (30, 0)
Size: 30 x 27
```

### Room Layout

```
^^^^^^^^^^^^^^^^^^^^^^^^^^
<   $$                   >
<                        >
<       $$      $$       >
<       $$      $$       >
<       $$      $$       >
<           m            >
<       $$      $$       >
<       $$      $$       >
<                        >
<       $$      $$       >
<                        >
<                        >
&&&&&&&&&&&&&&&&&&&&&&&&&&
```

---

## Leaf Node 17

```
Position: (60, 0)
Size: 33 x 27
```

### Room Layout

```
^^^^^^^^^^^^^^^^^^^^^^^^^^
<    $       $       $   >
<    $       $       $   >
<    $       $       $   >
<    $   $   $   $   $   >
<    $   $   $   $   $   >
<    $   $   $   $   $   >
<    $   $   $   $   $   >
<    $   $   $   $   $   >
<    $   $   $   $   $   >
<        $       $       >
<   m    $   l   $       >
<        $       $       >
&&&&&&&&&&&&&&&&&&&&&&&&&&
```

---

## Node 18

```
Position: (0, 27)
Size: 93 x 44
Left Child: 9
Right Child: 14
```

---

## Leaf Node 16

```
Position: (0, 27)
Size: 30 x 23
```

### Room Layout

```
^^^^^^^^^^^^^^^^^^^^^^^^^^
<  m                     >
<       $                >
<        $               >
<         $     m        >
<          $             >
<           $            >
<            $           >
<             $          >
<              $         >
<               $        >
<                $       >
<           t            >
&&&&&&&&&&&&&&&&&&&&&&&&&&
```

---

## Leaf Node 15

```
Position: (30, 27)
Size: 33 x 23
```

### Room Layout

```
^^^^^^^^^^^^^^^^^^^^^^^^^^
<   $$                   >
<                        >
<       $$      $$       >
<       $$      $$       >
<       $$      $$       >
<           m            >
<       $$      $$       >
<       $$      $$       >
<                        >
<       $$      $$       >
<                        >
<                        >
&&&&&&&&&&&&&&&&&&&&&&&&&&
```

---

## Leaf Node 15

```
Position: (63, 27)
Size: 30 x 23
```

### Room Layout

```
^^^^^^^^^^^^^^^^^^^^^^^^^^
<  m                     >
<       $                >
<        $               >
<         $     m        >
<          $             >
<           $            >
<            $           >
<             $          >
<              $         >
<               $        >
<                $       >
<           t            >
&&&&&&&&&&&&&&&&&&&&&&&&&&
```

---

## Leaf Node 16

```
Position: (0, 50)
Size: 58 x 21
```

### Room Layout

```
^^^^^^^^^^^^^^^^^^^^^^^^^^
<   $$                   >
<                        s
<       $$      $$       >
<       $$      $$       >
<       $$      $$       >
<   m       t            >
<       $$      $$       >
<       $$      $$       >
<                        >
<       $$      $$       >
<                        >
<                        >
&&&&&&&&&&&&&&&&&&&&&&&&&&
```

---

## Leaf Node 16

```
Position: (58, 50)
Size: 35 x 21
```

### Room Layout

```
^^^^^^^^^^^^^^^^^^^^^^^^^^
<   $$                   >
<                        >
<       $$      $$       >
<       $$      $$       >
<       $$      $$       >
<          tlt           >
<       $$      $$       >
<       $$      $$       >
<                        >
<       $$      $$       >
<                        >
<                        >
&&&&&&&&&&&&&&&&&&&&&&&&&&
```

---

## Right Subtree Root (Node 20)

```
Position: (93, 0)
Size: 57 x 100
Left Child: 23
Right Child: 28
```

---

## Leaf Node 17

```
Position: (93, 0)
Size: 57 x 27
```

### Room Layout

```
^^^^^^^^^^^^^^^^^^^^^^^^^^
<   $$                   >
<                        >
<       $$      $$       >
<       $$      $$       >
<       $$      $$       >
<           m            >
<       $$      $$       >
<       $$      $$       >
<                        >
<       $$      $$       >
<                        >
<                        >
&&&&&&&&&&&&&&&&&&&&&&&&&&
```

---

## Leaf Node 17

```
Position: (93, 27)
Size: 57 x 23
```

### Room Layout

```
^^^^^^^^^^^^^^^^^^^^^^^^^^
<   $$                   >
<                        s
<       $$      $$       >
<       $$      $$       >
<       $$      $$       >
<   m       t            >
<       $$      $$       >
<       $$      $$       >
<                        >
<       $$      $$       >
<                        >
<                        >
&&&&&&&&&&&&&&&&&&&&&&&&&&
```

---

## Leaf Node 18

```
Position: (93, 50)
Size: 57 x 24
```

### Room Layout

```
^^^^^^^^^^^^^^^^^^^^^^^^^^
<   $$                   >
<                        >
<       $$      $$       >
<       $$      $$       >
<       $$      $$       >
<           m            >
<       $$      $$       >
<       $$      $$       >
<                        >
<       $$      $$       >
<                        >
<                        >
&&&&&&&&&&&&&&&&&&&&&&&&&&
```

---

## Leaf Node 19

```
Position: (93, 74)
Size: 57 x 26
```

### Room Layout

```
^^^^^^^^^^^^^^^^^^^^^^^^^^
<   $$                   >
<                        >
<       $$      $$       >
<       $$      $$       >
<       $$      $$       >
<          tlt           >
<       $$      $$       >
<       $$      $$       >
<                        >
<       $$      $$       >
<                        >
<                        >
&&&&&&&&&&&&&&&&&&&&&&&&&&
```

---

# Notes

* Nodes with `left_child = -1` and `right_child = -1` are **leaf nodes (rooms)**
* ASCII blocks represent generated room layouts
* Coordinates are **relative to world grid origin**
* Tree structure follows **BSP partitioning hierarchy**

---
