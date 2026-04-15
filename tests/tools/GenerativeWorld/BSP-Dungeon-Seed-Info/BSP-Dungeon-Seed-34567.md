# BSP Dungeon Generation Dump (Given The BSP is generated with seed 34567)
# Data information generated from BSP-Dungeon's TreeParser() function, Formatted with help from ChatGPT

###### Some clarification

The number next to the node represent the hierarchy in the tree. The highest (#) value of the node (ex. Node #) is the root node
and the descending (#) of the Nodes represent the depth level of node. Nodes of the same (#) value are on the same depth level.

---

# Summary

* **Total Nodes:** 29
* **Leaf Nodes:** 15


```

## High Level Abstraction representation of the tree

# BSP Tree (Clean View)

---

Container (Generated with seed 34567)
├── Top (Node 20)
│   ├── 19
│   │   ├── 18
│   │   │   ├── 17
│   │   │   └── 17
│   │   └── 18
│   │       ├── 17
│   │       └── 17
│   └── 19
│       ├── 18
│       │   ├── 17
│       │   └── 17
│       └── 18
│           ├── 17
│           └── 17
│
└── Bottom (Node 20)
    ├── 19
    │   ├── 18
    │   │   ├── 17
    │   │   └── 17
    │   └── 18
    └── 19
        ├── 18
        └── 18
            ├── 17
            └── 17
                ├── 16
                └── 16
```

------------
Name Container
x 0
y 0
width 150
height 100
left_child, [Index]: 1
right_child, [Index]: 16
------------
Name 20
x 0
y 0
width 150
height 59
left_child, [Index]: 2
right_child, [Index]: 9
------------
Name 19
x 0
y 0
width 66
height 59
left_child, [Index]: 3
right_child, [Index]: 6
------------
Name 18
x 0
y 0
width 34
height 59
left_child, [Index]: 4
right_child, [Index]: 5
------------
Name 17
x 0
y 0
width 34
height 37
left_child, [Index]: -1
right_child, [Index]: -1
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
------------
Name 17
x 0
y 37
width 34
height 22
left_child, [Index]: -1
right_child, [Index]: -1
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
------------
Name 18
x 34
y 0
width 32
height 59
left_child, [Index]: 7
right_child, [Index]: 8
------------
Name 17
x 34
y 0
width 32
height 32
left_child, [Index]: -1
right_child, [Index]: -1
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
------------
Name 17
x 34
y 32
width 32
height 27
left_child, [Index]: -1
right_child, [Index]: -1
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
------------
Name 19
x 66
y 0
width 84
height 59
left_child, [Index]: 10
right_child, [Index]: 13
------------
Name 18
x 66
y 0
width 84
height 31
left_child, [Index]: 11
right_child, [Index]: 12
------------
Name 17
x 66
y 0
width 45
height 31
left_child, [Index]: -1
right_child, [Index]: -1
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
------------
Name 17
x 111
y 0
width 39
height 31
left_child, [Index]: -1
right_child, [Index]: -1
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
------------
Name 18
x 66
y 31
width 84
height 28
left_child, [Index]: 14
right_child, [Index]: 15
------------
Name 17
x 66
y 31
width 37
height 28
left_child, [Index]: -1
right_child, [Index]: -1
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
------------
Name 17
x 103
y 31
width 47
height 28
left_child, [Index]: -1
right_child, [Index]: -1
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
------------
Name 20
x 0
y 59
width 150
height 41
left_child, [Index]: 17
right_child, [Index]: 22
------------
Name 19
x 0
y 59
width 150
height 20
left_child, [Index]: 18
right_child, [Index]: 21
------------
Name 18
x 0
y 59
width 110
height 20
left_child, [Index]: 19
right_child, [Index]: 20
------------
Name 17
x 0
y 59
width 56
height 20
left_child, [Index]: -1
right_child, [Index]: -1
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
------------
Name 17
x 56
y 59
width 54
height 20
left_child, [Index]: -1
right_child, [Index]: -1
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
------------
Name 18
x 110
y 59
width 40
height 20
left_child, [Index]: -1
right_child, [Index]: -1
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
------------
Name 19
x 0
y 79
width 150
height 21
left_child, [Index]: 23
right_child, [Index]: 24
------------
Name 18
x 0
y 79
width 41
height 21
left_child, [Index]: -1
right_child, [Index]: -1
^^^^^^^^^^^^^^^^^^^^^^^^^^
<                        >
<    $$$$$$$$$$$$$$$$$   >
<    $     lllll     $   >
<    $               $   >
<    $$$$$$$   $$$$$$$   >
<                        >
&&&&&&&&&&&&&&&&&&&&&&&&&&
------------
Name 18
x 41
y 79
width 109
height 21
left_child, [Index]: 25
right_child, [Index]: 26
------------
Name 17
x 41
y 79
width 45
height 21
left_child, [Index]: -1
right_child, [Index]: -1
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
------------
Name 17
x 86
y 79
width 64
height 21
left_child, [Index]: 27
right_child, [Index]: 28
------------
Name 16
x 86
y 79
width 32
height 21
left_child, [Index]: -1
right_child, [Index]: -1
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
------------
Name 16
x 118
y 79
width 32
height 21
left_child, [Index]: -1
right_child, [Index]: -1
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

