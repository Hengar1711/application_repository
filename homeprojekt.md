Modding interface version 1


    #pragma once

    #define DYNALO_EXPORT_SYMBOLS
    #include "../third/dynalo/symbol_helper.hpp"


    #include <cstdint>
    #include <unordered_map>
    #include <memory>

    class EventBase
    {
    public:
        EventBase() = default;
        virtual ~EventBase()  = default;
        /*!
        @brief 
        
        @param data Hier wird die Sturktur Übergeben, welche die Referenzdaten übergeben
        @return true 
        @return false 
        */
        virtual bool init(void *data) = 0;
        virtual void run() = 0;
        virtual void print() = 0;
        virtual void destroy() = 0;
        /*!
            @brief Get the raw object
            
            @param var 
            @return void* 
        */
        virtual void* get_raw(const char *var) = 0;
        /*!
            @brief Die Funktion erwartet die Cast Klasse Stand Alone und gibt den Zugrif als Pointer zurückd
            
            @tparam T 
            @param var 
            @return T* 
        */
        template <typename T>
        T* get(const char* var)
        {
            return reinterpret_cast<T*>(get_raw(var));
        };

        /*!
            @brief Set the raw object
            
            @param var Der Vorbereitete Key
            @param content 
            @return true 
            @return false 
        */
        virtual bool set(const char *var, void* content) = 0;
    };


    class TestCalc
    {
    public:
        int *Ref;
        int Cal;
        
        TestCalc(int *Ref, int Cal): Ref(Ref), Cal(Cal) {};
    };



use example


    #include "JackInTheBox.hpp"
    #include <iostream>

    JackInTheBox::JackInTheBox()
    {
        
    };
    JackInTheBox::~JackInTheBox()
    {
        Basvalue = nullptr;
        Manager = nullptr;
    };

    bool JackInTheBox::init(void *data)
    {
        auto Data = static_cast<TestCalcWith*>(data);

        Basvalue = Data->Ref;

        Manager = Data->Manager;
        Libmanager = Data->Libmanager;

        return true;
    };

    void JackInTheBox::run()
    {
        TestCalc Buffer1(Basvalue, 6);
        TestCalc Buffer2(Basvalue, 7);
        TestCalc Buffer3(Basvalue, 4);
        TestCalc Buffer4(Basvalue, 2);

        EventBase* BufferEvent1 = Libmanager->Manager["libaddition.dll"].createEvent();
        EventBase* BufferEvent2 = Libmanager->Manager["libaddition.dll"].createEvent();
        EventBase* BufferEvent3 = Libmanager->Manager["libsubtraction.dll"].createEvent();
        EventBase* BufferEvent4 = Libmanager->Manager["libaddition.dll"].createEvent();

        std::shared_ptr<EventBase> Addition1(BufferEvent1,[](EventBase* ptr){ if(ptr){ptr->destroy();}});
        std::shared_ptr<EventBase> Addition2(BufferEvent2,[](EventBase* ptr){ if(ptr){ptr->destroy();}});
        std::shared_ptr<EventBase> Substraktion(BufferEvent3,[](EventBase* ptr){ if(ptr){ptr->destroy();}});
        std::shared_ptr<EventBase> Addition3(BufferEvent4,[](EventBase* ptr){ if(ptr){ptr->destroy();}});

        Addition1->init(&Buffer1);
        Addition2->init(&Buffer2);
        Substraktion->init(&Buffer3);
        Addition3->init(&Buffer4);

        Manager->instanzing(std::move(Addition1));
        Manager->instanzing(std::move(Addition2));
        Manager->instanzing(std::move(Substraktion));
        Manager->instanzing(std::move(Addition3));
    };
    void JackInTheBox::print()
    {
        std::cout << "4 Berechnungen eingebunden." << std::endl;
    };

    void JackInTheBox::destroy()
    {
        delete this;
    };

    void* JackInTheBox::get_raw(const char *var)
    {
        return nullptr;
    };

    bool JackInTheBox::set(const char *var, void* content)
    {
        return true;
    };

    DYNALO_EXPORT EventBase* DYNALO_CALL create_event()
    {
        EventBase* out = new JackInTheBox;

        return out;
    };








catched? ;)

version 2 ++==  

init extension -> magic c struct  

agianst size of, variable count and hash code