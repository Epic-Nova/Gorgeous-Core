# 🔎 About Object Variables

???+ info "Short Description"

    The Gorgeous Object Variable system provides a flexible and extensible way to define variables as objects within the Gorgeous Things ecosystem, supporting various data types and offering features like network replication and dynamic property management. 

??? info "Long Description"

    The Gorgeous Object Variable system allows developers to define variables as UObjects, offering a powerful and organized approach to data storage and manipulation in Unreal Engine.
    This system supports single variables as well as arrays, maps, and sets, providing a wide range of data structuring capabilities. 

    Key features include:

    * **Base for Object Variables:** Serves as the foundation for creating specialized variable types.
    * **Dynamic Data Storage:** Allows for storing and manipulating various data types as UObjects.
    * **Network Replication:** Supports network replication for synchronized variable states.
    * **Getter and Setter Interfaces:** Integrates interfaces for accessing and modifying variable values.
    * **Dynamic Property Management:** Enables setting and getting properties dynamically using templates.
    * **Object Variable Registry:** Tracks all active object variables. 
    * **Persistence:** Supports persisting variables across level transitions. 
    * **Unique Identification:** Generates a unique ID for each object variable instance.

## 🤔 Frequently Asked Questions (FAQ)

**Question:** What types of variables does the Gorgeous Object Variable system support?

**Answer:** It supports single variables, arrays, maps, and sets, allowing for flexible data organization. 

**Question:** Does the Object Variable system handle network replication?

**Answer:** Yes, it supports network replication to ensure synchronized variable states across clients.