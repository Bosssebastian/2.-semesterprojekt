<script lang="ts">
	import { cn, type WithElementRef } from "$lib/utils.js";
	import type { HTMLAttributes } from "svelte/elements";

	type Props = WithElementRef<HTMLAttributes<HTMLDivElement>, HTMLDivElement>;

	let { class: className, ref = $bindable(null), children, ...restProps }: Props = $props();
</script>

<div
	bind:this={ref}
	data-slot="dialog-header"
	class={cn("flex flex-col gap-3 text-left", className)}
	{...restProps}
>
	{@render children?.()}
</div>
