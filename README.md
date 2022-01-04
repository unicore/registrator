# REG
Стек: C/C++, eosio.cdt 1.5.0

# Введение
Поскольку каждый новый аккаунт системы должен получить ресурсы RAM, CPU и NET, а количество ресурсов в системе ограничено, то все регистрации являются платными и за них кто-то должен заплатить в системном токене. Регистратор обеспечивает работу FREEMIUM бизнес-модели, при которой, пользователь получает возможность ограниченного использования любого продукта системы до истечения срока давности пользования своим гостевым аккаунтом. Регистратором нового аккаунта может выступить любой аккаунт системы и получить комиссию за это. 

# Компиляция
Заменить ABSOLUTE_PATH_TO_CONTRACT на абсолютный путь к директории контракта reg. 
```
docker run --rm --name eosio.cdt_v1.5.0 --volume /ABSOLUTE_PATH_TO_CONTRACT:/project -w /project eostudio/eosio.cdt:v1.5.0 /bin/bash -c "eosio-cpp -abigen -I include -R include -contract reg -o reg.wasm registrator.cpp" & 

```

# Установка
```
cleos set contract registrator ABSOLUTE_PATH_TO_CONTRACT -p registrator
```

# Действующие аккаунты
- me - собственное имя аккаунта контракта
- partners - имя аккаунта контракта хранилища партнёров
- core - имя аккаунта цифровой экономики ядра
- system_account - имя аккаунта системного контракта    

# Основные управляющие параметры
- GUEST_EXPIRATION - продолжительность гостевого периода, после которого, аккаунт может быть отозван
- SYMBOL - системный токен
- ramcore_symbol - идентификационный токен рынка оперативной памяти
- RAM_symbol - токен рынка оперативной памяти
- MIN_AMOUNT - комиссия, взымаемая регистратором за пользование гостевым аккаунтом
- BASKET - количество гостевых аккаунтов, которые могут быть отозваны за один вызов действия
    

# Роли
- Пользователь. Гость или партнёр, чей аккаунт регистрируется. 
- Регистратор. Тот, кто регистрирует аккаунт и оплачивает ресурсы аккаунта. 

# Пополнение баланса
Для совершения действий регистрации аккаунта, необходимо обладать системным токеном на своём балансе в регистраторе. Для пополнения баланса, необходимо отправить перевод на аккаунт контракта в системном токене с указанием регистратора в поле memo: 
```
cleos transfer username registrator "100.0000 FLOWER" "registrator"
```
После чего можем получить баланс регистратора из таблицы:
```
cleos get table registrator registrator balance
```
В ответе будет баланс:
```
"rows": [{
      "username": "username",
      "quantity": "100.0000 FLOWER"
    }
  ],
```

# Регистрация гостя
Для регистрации аккаунта в качестве гостя, необходимо вызвать метод regaccount, передав параметр is_guest = true: 
```
cleos push action registrator regaccount '[username, "", tester, EOS6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV, "1.0000 FLOWER", "1.0000 FLOWER", 16384, true, false]' -p username
```
Тогда, получив аккаунт tester, мы увидим, что права владельца находятся у регистратора:
```
permissions: 
     owner     1:    1 registrator@eosio.code, 
        active     1:    1 EOS6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV
memory: 
     quota:        16 KiB    used:     2.908 KiB  

net bandwidth: 
     delegated:    1.0000 FLOWER           (total staked delegated to account from others)
     used:                 0 bytes
     available:        3.169 TiB  
     limit:            3.169 TiB  

cpu bandwidth:
     delegated:    1.0000 FLOWER           (total staked delegated to account from others)
     used:                 0 us   
     available:        92.31 hr   
     limit:            92.31 hr   
```

# Регистрация партнёра
Для регистрации аккаунта в качестве гостя, необходимо вызвать метод regaccount, передав параметр is_guest = false: 
```
cleos push action registrator regaccount '[username, "", tester2, EOS6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV, "1.0000 FLOWER", "1.0000 FLOWER", 16384, true, false]' -p username
```

Тогда, получив аккаунт tester2, мы увидим, что права владельца находятся у владельца ключа:
```
permissions: 
     owner     1:    1 EOS6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV
        active     1:    1 EOS6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV
memory: 
     quota:        16 KiB    used:     3.365 KiB  

net bandwidth: 
     staked:       1.0000 FLOWER           (total stake delegated from account to self)
     delegated:    0.0000 FLOWER           (total staked delegated to account from others)
     used:                 0 bytes
     available:        3.109 TiB  
     limit:            3.109 TiB  

cpu bandwidth:
     staked:       1.0000 FLOWER           (total stake delegated from account to self)
     delegated:    0.0000 FLOWER           (total staked delegated to account from others)
     used:                 0 us   
     available:        90.57 hr   
     limit:            90.57 hr    
```

# Отзыв аккаунта гостя
Для того, чтобы отозвать аккаунт у гостя, необходимо вызвать метод update. Метод получает таблицу guests с сортировкой по ключу expiration_at, и заменяет активные ключи аккаунтов на себя. Количество аккаунтов к отзыву за один вызов метода update регулируется константой BASKET. Вызов метода может осуществить любой аккаунт сети. 
```
cleos push action registrator update '[]' -p tester
```
Получив объект аккаунта истекшего гостя, увидим, что активные права доступа теперь принадлежат регистратору: 
```
permissions: 
     owner     1:    1 registrator@eosio.code, 
        active     1:    1 registrator@eosio.code, 
memory: 
     quota:        16 KiB    used:     2.891 KiB  

net bandwidth: 
     delegated:    1.0000 FLOWER           (total staked delegated to account from others)
     used:                 0 bytes
     available:        3.109 TiB  
     limit:            3.109 TiB  

cpu bandwidth:
     delegated:    1.0000 FLOWER           (total staked delegated to account from others)
     used:                 0 us   
     available:        90.57 hr   
     limit:            90.57 hr 
```
Все отозванные аккаунты хрянятся в таблице reserved: 
```
cleos get table registrator registrator reserved
```
В ответе список отозванных аккаунтов с указанием регистраторов:
```
"rows": [{
      "username": "eyfyjkpbctwv",
      "registrator": "username"
    },{
      "username": "hgrmogbyerml",
      "registrator": "username"
    },{
      "username": "yotlnhctihdg",
      "registrator": "username"
    }
  ]
```

# TODO
- Рассмотреть возможность внедрения бизнес-модель доходного пула ликвидности для аккаунтов, обеспечивающих регистрацию гостей. Сейчас каждый регистратор обеспечивает оплату стоимости аккаунта собственными средствами. Предоставить возможность собирать средства на оплату новых регистраций из числа средств партнёров. 
- Внедрить систему оценки и выдачу коротких имён.