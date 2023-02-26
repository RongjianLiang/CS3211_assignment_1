# include <mutex>
# include <memory>
# include <functional>

template <typename T> 
class List
{
    struct node 
    {
        std::mutex node_mutex;
        std::shared_ptr<T> data;
        std::unique_ptr<node> next;
        node():next(){}
        node(T const& value):
            data(std::make_shared<T>(value)){}

    };
    node head;

public:
    List(){}
    ~List()
    {
        remove_if([](node const&){return true;});
    }
    List(List const& other) = delete;
    List& operator=(List const& other) = delete;

    void push_front(T const& value)
    {
        std::unique_ptr<node> new_node(new node(value));
        std::lock_guard<std::mutex> lk(head.node_mutex);
        new_node->next = std::move(head.next);
        head.next = std::move(new_node);
    }

    // // push the new value at the right place, so that the list is in ascending orders 
    // // the comparator should return false if new_node is bigger than pre_node
    // // void push_at(T const& value, std::function<bool(node&, node&)> compare)
    // template<typename Function>
    // void push_at(T const& value, Function f)
    // {
    //     node* pre_node = &head; 
        
    //     std::unique_lock<std::mutex> lk(head.node_mutex);
    //     while(node* const curr = pre_node->next.get())
    //     {
    //         if(f(temp, pre_node->data) == false)
    //         {
    //             std::unique_ptr<node> new_node(new node(value));
    //             new_node->next = pre_node->next;
    //             pre_node->next = std::move(new_node);
    //             lk.unlock();
    //             break;
    //         }
    //         else
    //         {
    //             continue;
    //         }
    //     }
    // }

    template<typename Function>
    void for_each(Function f)
    {
        node* current = &head;
        std::unique_lock<std::mutex> lk(head.node_mutex);
        while(node* const next = current->next.get())
        {
            std::unique_lock<std::mutex> next_lk (next->node_mutex);
            lk.unlock();
            f (*next->data);
            current = next;
            lk = std::move(next_lk);
        }
    }

    template<typename Predicate>
    std::shared_ptr<T> find_first_if(Predicate p)
    {
        node* current = &head;
        std::unique_lock<std::mutex> lk(head.node_mutex);
        while(node* const next = current->next.get())
        {
            std::unique_lock<std::mutex> next_lk(next->node_mutex);
            lk.unlock();
            if(p(*next->data))
            {
                return next->data;
            }
            current = next;
            lk = std::move(next_lk);
        }
        return std::shared_ptr<T>();
    }

    template<typename Predicate>
    void remove_if(Predicate p)
    {
        node* current = &head;
        std::unique_lock<std::mutex> lk(head.node_mutex);
        while(node* const next = current->next.get())
        {
            std::unique_lock<std::mutex> next_lk(next->node_mutex);
            if(p(*next->data))
            {
                std::unique_ptr<node> old_next = std::move(current->next);
                current->next = std::move(next->next);
                next_lk.unlock();
            }
            else
            {
                lk.unlock();
                current = next;
                lk = std::move(next_lk);
            }
        }
    }
    // this only swaps a given node with its next node
    void swap_with_next(node& a)
    {
        node* pre_node = &head;
        std::unique_lock<std::mutex> pre_lk(head.node_mutex);
        while(node* const curr_node = pre_node->next.get())
        {
            std::unique_lock<std::mutex> curr_lk(curr_node->node_mutex);
            if(curr_node == a)
            {
                pre_node->next = curr_node->next;
                node* next_node = &(curr_node->next);
                std::unique_lock<std::mutex> next_lk(next_node->node_mutex);
                pre_lk.unlock();
                curr_node->next = next_node->next;
                next_node->next = curr_node;
                curr_lk.unlock();
                next_lk.unlock();
            }
            else
            {
                pre_lk.unlock();
                pre_node = curr_node;
                pre_lk = std::move(curr_lk);
            }
        }
    }
};