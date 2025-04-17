from datasets import load_dataset
import sys
from tqdm import tqdm
import git

dataset_info_dict = {
    "molecules" : {"url" : "HoangHa/belka-smiles-train-raw", "filename" : "molecules", "column_filter_name" : "protein_name", "column_filter_name" : "BRD4", "column_value" : "molecule_smiles"},
}

def get_git_root():
    repo = git.Repo('.', search_parent_directories=True)
    return repo.working_tree_dir

if __name__ == "__main__":
    dataset_input = str(sys.argv[1])
    if dataset_input not in dataset_info_dict:
        raise NotImplementedError("Name of dataset not found") 
    dataset_info = dataset_info_dict[dataset_input]
    file_root = get_git_root() + "/data/temp/"
    current_dataset = dataset_info["url"]
    dataset = load_dataset(current_dataset)
    print("Loading Complete")
    shuffled_dataset = dataset.shuffle(seed=42)
    print("Shuffle Complete")
    n = int(input("Number of Samples to Save (empty to exit): "))
    while n != 0:
        print(f"Writing {n} samples")
        pbar = tqdm(total=n)
        with open(f"{dataset_info["filename"]}-{n}.txt", "w+") as f:
            i = 1
            for example in shuffled_dataset["train"]:
                if example[dataset_info["column_filter_name"]] == dataset_info["column_filter_name"]:
                    f.write(dataset_info["coumn_value"] + "\n")
                    if i >= n:
                        break
                    i+=1
                    pbar.update(1)
        
        pbar.close()
        n = int(input("Number of Samples to Save (empty to exit): "))


