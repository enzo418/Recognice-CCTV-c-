# Frontend
The fronted is built dynamically. First we get the html-elements specifications for the camera and program configurations from "elements.json", then we get the translations from "translations.json". Then the users choices a configuration file and the program parses that in order to acquire the defaults value for each field. After that the program generates the elements from differents templates.

### **elements.json**
In this file are all the elements/fields of the program and camera configuration. It's a json that his root object has two properties, `program` and `camera`, each one has a `groups` property that contains the groups. Each of these `group` has its `name` and the `elements` it contains, it can also have `groups`.

A element has 5 properties:
- `target`: string
    - target filed to change in the configuration.

- `type`: string with number|integer|int|decimal | checkbox|boolean|bool | text|string
    - the input type of the html element

- `hidden`: boolean

- `placeholder`: string
    - only for string or numeric inputs

- `min`: number
    - only for numeric inputs

- `max`: number
    - only for numeric inputs

### **translations.json**
As its name describes, it contains the translations.

Its root object contains all the language (code of 2 letters) supported.
Each language contains all the elements with its `label` and `description` translated.